//! Parses Rust sources with `syn` and emits Sourcetrail nodes, edges and locations.
//!
//! Two passes: the first builds a crate-wide symbol table (needed to resolve
//! cross-file references and method calls), the second emits for the requested
//! files. Both passes run over the same parsed ASTs.

use crate::db::{self, Db, Range};
use anyhow::{Context, Result};
use std::collections::HashMap;
use std::path::{Path, PathBuf};
use syn::spanned::Spanned;

const PRIMITIVES: &[&str] = &[
    "bool", "char", "f32", "f64", "i8", "i16", "i32", "i64", "i128", "isize", "str", "u8", "u16",
    "u32", "u64", "u128", "usize", "()",
];

pub struct CrateIndex {
    pub name: String,
    pub src_root: PathBuf,
    pub files: Vec<PathBuf>,
    asts: HashMap<PathBuf, (syn::File, String)>,
    /// fully qualified name -> node kind
    defs: HashMap<String, i32>,
    /// bare name -> fully qualified names carrying it
    by_name: HashMap<String, Vec<String>>,
    /// method name -> fully qualified names of matching methods
    methods: HashMap<String, Vec<String>>,
    /// fqn -> (name prefix, name postfix), e.g. return type and parameter list.
    /// Definitions and references must agree on these or Sourcetrail sees two
    /// separate symbols, so they live here rather than being recomputed.
    sigs: HashMap<String, (String, String)>,
}

impl CrateIndex {
    /// `src_root` is the crate's `src` directory, `name` its Cargo package name
    /// with dashes normalised to underscores.
    pub fn scan(src_root: &Path, name: &str) -> Result<Self> {
        let mut files: Vec<PathBuf> = walkdir::WalkDir::new(src_root)
            .into_iter()
            .filter_map(|e| e.ok())
            .filter(|e| e.file_type().is_file() && e.path().extension().is_some_and(|x| x == "rs"))
            .map(|e| e.into_path())
            .collect();
        files.sort();

        let mut me = CrateIndex {
            name: name.to_string(),
            src_root: src_root.to_path_buf(),
            files,
            asts: HashMap::new(),
            defs: HashMap::new(),
            by_name: HashMap::new(),
            methods: HashMap::new(),
            sigs: HashMap::new(),
        };

        for path in me.files.clone() {
            let text = std::fs::read_to_string(&path)
                .with_context(|| format!("reading {}", path.display()))?;
            match syn::parse_file(&text) {
                Ok(ast) => {
                    let module = me.module_path(&path);
                    {
                        let lines: Vec<&str> = text.lines().collect();
                        let r = Resolver::new(&me.name, module, &ast);
                        let mut scope = r.module.clone();
                        me.scan_items(&ast.items, &mut scope, &r, &lines);
                    }
                    me.asts.insert(path, (ast, text));
                }
                // A file we cannot parse is reported as an index error later, not fatal.
                Err(_) => {}
            }
        }
        Ok(me)
    }

    /// `src/commands/asset.rs` -> `["asset_bridge", "commands", "asset"]`,
    /// `src/lib.rs` and `src/commands/mod.rs` collapse onto their parent.
    pub fn module_path(&self, file: &Path) -> Vec<String> {
        let mut parts = vec![self.name.clone()];
        if let Ok(rel) = file.strip_prefix(&self.src_root) {
            let mut segs: Vec<String> = rel
                .components()
                .map(|c| c.as_os_str().to_string_lossy().to_string())
                .collect();
            if let Some(last) = segs.last_mut() {
                *last = last.trim_end_matches(".rs").to_string();
            }
            if matches!(segs.last().map(|s| s.as_str()), Some("mod" | "lib" | "main")) {
                segs.pop();
            }
            parts.extend(segs);
        }
        parts
    }

    fn add_def(&mut self, fqn: String, kind: i32) {
        if let Some(bare) = fqn.rsplit("::").next() {
            self.by_name.entry(bare.to_string()).or_default().push(fqn.clone());
            if kind == db::node::METHOD {
                self.methods.entry(bare.to_string()).or_default().push(fqn.clone());
            }
        }
        self.defs.insert(fqn, kind);
    }

    /// Pass one: record every definition's fully qualified name and kind.
    fn scan_items(
        &mut self,
        items: &[syn::Item],
        scope: &mut Vec<String>,
        r: &Resolver,
        lines: &[&str],
    ) {
        for item in items {
            match item {
                syn::Item::Mod(m) => {
                    let fqn = join(scope, &m.ident.to_string());
                    self.add_def(fqn, db::node::MODULE);
                    if let Some((_, inner)) = &m.content {
                        scope.push(m.ident.to_string());
                        self.scan_items(inner, scope, r, lines);
                        scope.pop();
                    }
                }
                syn::Item::Struct(s) => {
                    let fqn = join(scope, &s.ident.to_string());
                    self.add_def(fqn.clone(), db::node::STRUCT);
                    self.scan_fields(&s.fields, &fqn, lines);
                }
                syn::Item::Union(u) => {
                    let fqn = join(scope, &u.ident.to_string());
                    self.add_def(fqn.clone(), db::node::UNION);
                    self.scan_fields(&syn::Fields::Named(u.fields.clone()), &fqn, lines);
                }
                syn::Item::Enum(e) => {
                    let fqn = join(scope, &e.ident.to_string());
                    self.add_def(fqn.clone(), db::node::ENUM);
                    for v in &e.variants {
                        let vf = join(&[fqn.clone()], &v.ident.to_string());
                        self.add_def(vf.clone(), db::node::ENUM_CONSTANT);
                        self.scan_fields(&v.fields, &vf, lines);
                    }
                }
                syn::Item::Trait(t) => {
                    let fqn = join(scope, &t.ident.to_string());
                    self.add_def(fqn.clone(), db::node::INTERFACE);
                    for ti in &t.items {
                        match ti {
                            syn::TraitItem::Fn(f) => self.scan_fn(
                                join(&[fqn.clone()], &f.sig.ident.to_string()),
                                &f.sig,
                                db::node::METHOD,
                                lines,
                            ),
                            syn::TraitItem::Const(c) => self.scan_typed(
                                join(&[fqn.clone()], &c.ident.to_string()),
                                &c.ty,
                                db::node::GLOBAL_VARIABLE,
                                lines,
                            ),
                            syn::TraitItem::Type(t2) => {
                                self.add_def(join(&[fqn.clone()], &t2.ident.to_string()), db::node::TYPEDEF)
                            }
                            _ => {}
                        }
                    }
                }
                syn::Item::Impl(i) => {
                    let Some(owner) = r.self_ty_fqn(&i.self_ty, scope) else { continue };
                    for ii in &i.items {
                        match ii {
                            syn::ImplItem::Fn(f) => self.scan_fn(
                                join(&[owner.clone()], &f.sig.ident.to_string()),
                                &f.sig,
                                db::node::METHOD,
                                lines,
                            ),
                            syn::ImplItem::Const(c) => self.scan_typed(
                                join(&[owner.clone()], &c.ident.to_string()),
                                &c.ty,
                                db::node::GLOBAL_VARIABLE,
                                lines,
                            ),
                            syn::ImplItem::Type(t) => {
                                self.add_def(join(&[owner.clone()], &t.ident.to_string()), db::node::TYPEDEF)
                            }
                            _ => {}
                        }
                    }
                }
                syn::Item::Fn(f) => self.scan_fn(
                    join(scope, &f.sig.ident.to_string()),
                    &f.sig,
                    db::node::FUNCTION,
                    lines,
                ),
                syn::Item::Const(c) => self.scan_typed(
                    join(scope, &c.ident.to_string()),
                    &c.ty,
                    db::node::GLOBAL_VARIABLE,
                    lines,
                ),
                syn::Item::Static(s) => self.scan_typed(
                    join(scope, &s.ident.to_string()),
                    &s.ty,
                    db::node::GLOBAL_VARIABLE,
                    lines,
                ),
                syn::Item::Type(t) => {
                    self.add_def(join(scope, &t.ident.to_string()), db::node::TYPEDEF)
                }
                syn::Item::Macro(m) => {
                    if let Some(id) = &m.ident {
                        self.add_def(join(scope, &id.to_string()), db::node::MACRO);
                    }
                }
                _ => {}
            }
        }
    }

    fn scan_fields(&mut self, fields: &syn::Fields, owner: &str, lines: &[&str]) {
        for (i, f) in fields.iter().enumerate() {
            let name = f.ident.as_ref().map(|i| i.to_string()).unwrap_or_else(|| i.to_string());
            let fqn = join(&[owner.to_string()], &name);
            self.sigs.insert(fqn.clone(), (slice(lines, range_of(f.ty.span())), String::new()));
            self.add_def(fqn, db::node::FIELD);
        }
    }

    fn scan_fn(&mut self, fqn: String, sig: &syn::Signature, kind: i32, lines: &[&str]) {
        self.sigs.insert(fqn.clone(), sig_of(sig, lines));
        self.add_def(fqn, kind);
    }

    fn scan_typed(&mut self, fqn: String, ty: &syn::Type, kind: i32, lines: &[&str]) {
        self.sigs.insert(fqn.clone(), (slice(lines, range_of(ty.span())), String::new()));
        self.add_def(fqn, kind);
    }

    pub fn is_defined(&self, fqn: &str) -> Option<i32> {
        self.defs.get(fqn).copied()
    }
}

fn join(scope: &[String], name: &str) -> String {
    if scope.is_empty() {
        name.to_string()
    } else {
        format!("{}::{}", scope.join("::"), name)
    }
}

/// Maps names as written in one file onto fully qualified names.
pub struct Resolver {
    crate_name: String,
    pub module: Vec<String>,
    /// local alias -> fully qualified path
    uses: HashMap<String, String>,
    /// prefixes brought in by `use foo::*`
    globs: Vec<String>,
}

impl Resolver {
    pub fn new(crate_name: &str, module: Vec<String>, ast: &syn::File) -> Self {
        let mut r = Resolver {
            crate_name: crate_name.to_string(),
            module,
            uses: HashMap::new(),
            globs: Vec::new(),
        };
        for item in &ast.items {
            if let syn::Item::Use(u) = item {
                r.collect_use(&u.tree, &mut Vec::new());
            }
        }
        r
    }

    fn collect_use(&mut self, tree: &syn::UseTree, prefix: &mut Vec<String>) {
        match tree {
            syn::UseTree::Path(p) => {
                prefix.push(p.ident.to_string());
                self.collect_use(&p.tree, prefix);
                prefix.pop();
            }
            syn::UseTree::Name(n) => {
                let name = n.ident.to_string();
                let full = self.absolute(&join(prefix, &name));
                self.uses.insert(name, full);
            }
            syn::UseTree::Rename(rn) => {
                let full = self.absolute(&join(prefix, &rn.ident.to_string()));
                self.uses.insert(rn.rename.to_string(), full);
            }
            syn::UseTree::Glob(_) => {
                let full = self.absolute(&prefix.join("::"));
                self.globs.push(full);
            }
            syn::UseTree::Group(g) => {
                for t in &g.items {
                    self.collect_use(t, prefix);
                }
            }
        }
    }

    /// Rewrite `crate::`, `self::` and `super::` prefixes into absolute paths.
    fn absolute(&self, path: &str) -> String {
        let mut segs: Vec<&str> = path.split("::").collect();
        match segs.first().copied() {
            Some("crate") => {
                segs[0] = &self.crate_name;
                segs.join("::")
            }
            Some("self") => {
                segs.remove(0);
                join(&self.module, &segs.join("::"))
            }
            Some("super") => {
                let mut up = self.module.clone();
                while segs.first().copied() == Some("super") {
                    segs.remove(0);
                    up.pop();
                }
                join(&up, &segs.join("::"))
            }
            _ => segs.join("::"),
        }
    }

    /// Fully qualified name for a path as written, plus whether it is crate-local.
    /// `want` is the node kind the reference expects; it only constrains the
    /// last-resort bare-name lookup, where a wrong guess is otherwise silent.
    pub fn resolve(
        &self,
        path: &syn::Path,
        index: &CrateIndex,
        scope: &[String],
        want: i32,
    ) -> String {
        let segs: Vec<String> = path.segments.iter().map(|s| s.ident.to_string()).collect();
        if segs.is_empty() {
            return String::new();
        }
        let first = segs[0].as_str();

        if matches!(first, "crate" | "self" | "super") {
            return self.absolute(&segs.join("::"));
        }
        if first == "Self" {
            // `scope` already ends in the impl's type, so `Self::foo` is scope + foo.
            return if segs.len() == 1 { scope.join("::") } else { join(scope, &segs[1..].join("::")) };
        }
        if let Some(mapped) = self.uses.get(first) {
            let rest = &segs[1..];
            return if rest.is_empty() { mapped.clone() } else { format!("{mapped}::{}", rest.join("::")) };
        }
        // Defined in the enclosing scope, or anywhere in this module.
        let local = join(scope, &segs.join("::"));
        if index.is_defined(&local).is_some() {
            return local;
        }
        let in_module = join(&self.module, &segs.join("::"));
        if index.is_defined(&in_module).is_some() {
            return in_module;
        }
        // A glob import that resolves to exactly one known symbol.
        for g in &self.globs {
            let cand = format!("{g}::{}", segs.join("::"));
            if index.is_defined(&cand).is_some() {
                return cand;
            }
        }
        // Unique bare name anywhere in the crate, of a compatible kind. Without
        // the kind filter `format!` binds to a struct field called `format`.
        if segs.len() == 1 {
            if let Some(cands) = index.by_name.get(first) {
                let mut ok = cands.iter().filter(|c| {
                    let got = index.defs.get(*c).copied().unwrap_or(0);
                    if want == db::node::MACRO { got == db::node::MACRO } else { got != db::node::MACRO }
                });
                if let (Some(only), None) = (ok.next(), ok.next()) {
                    return only.clone();
                }
            }
        }
        // Leave external paths (`std::fs::read_to_string`) as written.
        segs.join("::")
    }

    /// Owner name of an `impl` block's self type.
    fn self_ty_fqn(&self, ty: &syn::Type, scope: &[String]) -> Option<String> {
        let syn::Type::Path(tp) = strip_refs(ty) else { return None };
        let segs: Vec<String> = tp.path.segments.iter().map(|s| s.ident.to_string()).collect();
        let first = segs.first()?.as_str();
        if matches!(first, "crate" | "self" | "super") {
            return Some(self.absolute(&segs.join("::")));
        }
        if let Some(mapped) = self.uses.get(first) {
            let rest = &segs[1..];
            return Some(if rest.is_empty() {
                mapped.clone()
            } else {
                format!("{mapped}::{}", rest.join("::"))
            });
        }
        Some(join(scope, &segs.join("::")))
    }
}

fn strip_refs(ty: &syn::Type) -> &syn::Type {
    match ty {
        syn::Type::Reference(r) => strip_refs(&r.elem),
        syn::Type::Paren(p) => strip_refs(&p.elem),
        syn::Type::Group(g) => strip_refs(&g.elem),
        _ => ty,
    }
}

pub fn range_of(span: proc_macro2::Span) -> Range {
    let s = span.start();
    let e = span.end();
    Range {
        start_line: s.line,
        start_col: s.column + 1,
        end_line: e.line,
        end_col: e.column.max(1),
    }
}

/// Exact source text covered by `r`, used for readable signatures in the UI.
fn slice(lines: &[&str], r: Range) -> String {
    if r.start_line == 0 || r.start_line > lines.len() {
        return String::new();
    }
    let out = if r.start_line == r.end_line {
        let l = lines[r.start_line - 1];
        let (a, b) = (r.start_col - 1, r.end_col.min(l.len()));
        if a >= b { String::new() } else { l[a..b].to_string() }
    } else {
        let mut parts = vec![lines[r.start_line - 1][(r.start_col - 1).min(lines[r.start_line - 1].len())..].to_string()];
        for l in &lines[r.start_line..(r.end_line - 1).min(lines.len())] {
            parts.push(l.trim().to_string());
        }
        if r.end_line <= lines.len() {
            let l = lines[r.end_line - 1];
            parts.push(l[..r.end_col.min(l.len())].to_string());
        }
        parts.join(" ")
    };
    out.split_whitespace().collect::<Vec<_>>().join(" ")
}

// ---------------------------------------------------------------------------
// Pass two: emit
// ---------------------------------------------------------------------------

pub struct Emitter<'a> {
    db: &'a mut Db,
    index: &'a CrateIndex,
    file_id: i64,
    lines: Vec<&'a str>,
    r: Resolver,
}

/// Index one file into `db`. Nodes are keyed by fully qualified name, so
/// Sourcetrail merges what separate files contribute about the same symbol.
pub fn emit_file(db: &mut Db, index: &CrateIndex, path: &Path) -> Result<()> {
    let Some((ast, text)) = index.asts.get(path) else {
        // Unparsable files were skipped during the scan; surface that in the UI.
        let text = std::fs::read_to_string(path).unwrap_or_default();
        let msg = syn::parse_file(&text)
            .err()
            .map(|e| format!("Rust parse error: {e}"))
            .unwrap_or_else(|| "file not indexed".to_string());
        let span = syn::parse_file(&text).err().map(|e| range_of(e.span())).unwrap_or(Range {
            start_line: 1,
            start_col: 1,
            end_line: 1,
            end_col: 1,
        });
        db.file(&path.to_string_lossy(), text.lines().count(), &text, &mtime(path))?;
        db.error(&msg, &path.to_string_lossy(), span)?;
        return Ok(());
    };

    let file_id = db.file(&path.to_string_lossy(), text.lines().count(), text, &mtime(path))?;
    let module = index.module_path(path);
    let r = Resolver::new(&index.name, module.clone(), ast);

    let mut e = Emitter { db, index, file_id, lines: text.lines().collect(), r };

    // Materialise the module chain so the crate shows up as a tree.
    let mut parent = 0i64;
    for depth in 1..=module.len() {
        let parts: Vec<&str> = module[..depth].iter().map(|s| s.as_str()).collect();
        let id = e.db.node(&db::serialize_name(&parts, "", ""), db::node::MODULE)?;
        e.db.define(id)?;
        if parent != 0 {
            e.db.edge(db::edge::MEMBER, parent, id)?;
        }
        parent = id;
    }
    // The file belongs to its module, which is what drives the file tree.
    e.db.edge(db::edge::MEMBER, parent, file_id)?;

    let mut scope = module;
    e.emit_uses(&ast.items, parent)?;
    e.emit_items(&ast.items, &mut scope, parent)?;
    Ok(())
}

fn mtime(path: &Path) -> String {
    let secs = std::fs::metadata(path)
        .and_then(|m| m.modified())
        .ok()
        .and_then(|t| t.duration_since(std::time::UNIX_EPOCH).ok())
        .map(|d| d.as_secs())
        .unwrap_or(0);
    // Sourcetrail stores "%Y-%m-%d %H:%M:%S" (see TimeStamp::toString).
    let days = secs / 86400;
    let (y, m, d) = civil_from_days(days as i64);
    let rem = secs % 86400;
    format!("{y:04}-{m:02}-{d:02} {:02}:{:02}:{:02}", rem / 3600, (rem % 3600) / 60, rem % 60)
}

/// Howard Hinnant's days-from-civil, inverted. Avoids pulling in a date crate.
fn civil_from_days(z: i64) -> (i64, u32, u32) {
    let z = z + 719468;
    let era = if z >= 0 { z } else { z - 146096 } / 146097;
    let doe = (z - era * 146097) as u64;
    let yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    let y = yoe as i64 + era * 400;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153;
    let d = (doy - (153 * mp + 2) / 5 + 1) as u32;
    let m = if mp < 10 { mp + 3 } else { mp - 9 } as u32;
    (if m <= 2 { y + 1 } else { y }, m, d)
}

/// Return type and parameter list as written in the source.
fn sig_of(sig: &syn::Signature, lines: &[&str]) -> (String, String) {
    let ret = match &sig.output {
        syn::ReturnType::Type(_, t) => slice(lines, range_of(t.span())),
        syn::ReturnType::Default => String::new(),
    };
    let params: Vec<String> = sig.inputs.iter().map(|a| slice(lines, range_of(a.span()))).collect();
    (ret, format!("({})", params.join(", ")))
}

impl<'a> Emitter<'a> {

    /// Node for a fully qualified name. The signature always comes from the
    /// crate-wide table, so a call site and the definition it refers to build
    /// the exact same serialized name and land on one node.
    fn node_for(&mut self, fqn: &str, fallback: i32) -> Result<i64> {
        if fqn.is_empty() {
            return Ok(0);
        }
        if PRIMITIVES.contains(&fqn) {
            return self.db.node(&db::serialize_name(&[fqn], "", ""), db::node::BUILTIN_TYPE);
        }
        let (prefix, postfix) = self.index.sigs.get(fqn).cloned().unwrap_or_default();
        let kind = self.index.is_defined(fqn).unwrap_or(fallback);
        let parts: Vec<&str> = fqn.split("::").collect();
        self.db.node(&db::serialize_name(&parts, &prefix, &postfix), kind)
    }

    fn define_node(&mut self, fqn: &str, kind: i32) -> Result<i64> {
        let id = self.node_for(fqn, kind)?;
        self.db.define(id)?;
        Ok(id)
    }

    fn at(&mut self, element: i64, span: proc_macro2::Span, kind: i32) -> Result<()> {
        let file_id = self.file_id;
        self.db.location(element, file_id, range_of(span), kind)
    }

    fn emit_uses(&mut self, items: &[syn::Item], module_node: i64) -> Result<()> {
        for item in items {
            let syn::Item::Use(u) = item else { continue };
            let mut targets = Vec::new();
            collect_use_paths(&u.tree, &mut Vec::new(), &mut targets);
            for t in targets {
                let fqn = self.r.absolute(&t);
                let id = self.node_for(&fqn, db::node::SYMBOL)?;
                if id != 0 {
                    self.db.edge(db::edge::IMPORT, module_node, id)?;
                    self.at(id, u.span(), db::loc::TOKEN)?;
                }
            }
        }
        Ok(())
    }

    fn emit_items(&mut self, items: &[syn::Item], scope: &mut Vec<String>, parent: i64) -> Result<()> {
        for item in items {
            match item {
                syn::Item::Mod(m) => {
                    let fqn = join(scope, &m.ident.to_string());
                    let id = self.define_node(&fqn, db::node::MODULE)?;
                    self.db.edge(db::edge::MEMBER, parent, id)?;
                    self.at(id, m.ident.span(), db::loc::TOKEN)?;
                    if let Some((_, inner)) = &m.content {
                        self.at(id, m.span(), db::loc::SCOPE)?;
                        scope.push(m.ident.to_string());
                        self.emit_items(inner, scope, id)?;
                        scope.pop();
                    }
                }
                syn::Item::Struct(s) => {
                    let fqn = join(scope, &s.ident.to_string());
                    let id = self.define_node(&fqn, db::node::STRUCT)?;
                    self.db.edge(db::edge::MEMBER, parent, id)?;
                    self.at(id, s.ident.span(), db::loc::TOKEN)?;
                    self.at(id, s.span(), db::loc::SCOPE)?;
                    self.emit_fields(&s.fields, &fqn, id, scope)?;
                }
                syn::Item::Union(u) => {
                    let fqn = join(scope, &u.ident.to_string());
                    let id = self.define_node(&fqn, db::node::UNION)?;
                    self.db.edge(db::edge::MEMBER, parent, id)?;
                    self.at(id, u.ident.span(), db::loc::TOKEN)?;
                    self.at(id, u.span(), db::loc::SCOPE)?;
                    self.emit_fields(&syn::Fields::Named(u.fields.clone()), &fqn, id, scope)?;
                }
                syn::Item::Enum(en) => {
                    let fqn = join(scope, &en.ident.to_string());
                    let id = self.define_node(&fqn, db::node::ENUM)?;
                    self.db.edge(db::edge::MEMBER, parent, id)?;
                    self.at(id, en.ident.span(), db::loc::TOKEN)?;
                    self.at(id, en.span(), db::loc::SCOPE)?;
                    for v in &en.variants {
                        let vf = join(&[fqn.clone()], &v.ident.to_string());
                        let vid = self.define_node(&vf, db::node::ENUM_CONSTANT)?;
                        self.db.edge(db::edge::MEMBER, id, vid)?;
                        self.at(vid, v.ident.span(), db::loc::TOKEN)?;
                        self.at(vid, v.span(), db::loc::SCOPE)?;
                        self.emit_fields(&v.fields, &vf, vid, scope)?;
                    }
                }
                syn::Item::Trait(t) => {
                    let fqn = join(scope, &t.ident.to_string());
                    let id = self.define_node(&fqn, db::node::INTERFACE)?;
                    self.db.edge(db::edge::MEMBER, parent, id)?;
                    self.at(id, t.ident.span(), db::loc::TOKEN)?;
                    self.at(id, t.span(), db::loc::SCOPE)?;
                    for sup in &t.supertraits {
                        if let syn::TypeParamBound::Trait(tb) = sup {
                            let target = self.r.resolve(&tb.path, self.index, scope, db::node::INTERFACE);
                            let tid = self.node_for(&target, db::node::INTERFACE)?;
                            self.db.edge(db::edge::INHERITANCE, id, tid)?;
                            self.at(tid, tb.path.span(), db::loc::TOKEN)?;
                        }
                    }
                    scope.push(t.ident.to_string());
                    for ti in &t.items {
                        if let syn::TraitItem::Fn(f) = ti {
                            let mfqn = join(&[fqn.clone()], &f.sig.ident.to_string());
                            let mid = self.emit_signature(&f.sig, &mfqn, db::node::METHOD, scope)?;
                            self.db.edge(db::edge::MEMBER, id, mid)?;
                            self.at(mid, f.span(), db::loc::SCOPE)?;
                            if let Some(block) = &f.default {
                                self.emit_body(mid, &f.sig, Some(block), scope)?;
                            }
                        }
                    }
                    scope.pop();
                }
                syn::Item::Impl(i) => self.emit_impl(i, scope, parent)?,
                syn::Item::Fn(f) => {
                    let fqn = join(scope, &f.sig.ident.to_string());
                    let id = self.emit_signature(&f.sig, &fqn, db::node::FUNCTION, scope)?;
                    self.db.edge(db::edge::MEMBER, parent, id)?;
                    self.at(id, f.span(), db::loc::SCOPE)?;
                    self.emit_body(id, &f.sig, Some(&f.block), scope)?;
                }
                syn::Item::Const(c) => {
                    let fqn = join(scope, &c.ident.to_string());
                    let id = self.define_node(&fqn, db::node::GLOBAL_VARIABLE)?;
                    self.db.edge(db::edge::MEMBER, parent, id)?;
                    self.at(id, c.ident.span(), db::loc::TOKEN)?;
                    self.at(id, c.span(), db::loc::SCOPE)?;
                    self.emit_type(&c.ty, id, scope)?;
                }
                syn::Item::Static(s) => {
                    let fqn = join(scope, &s.ident.to_string());
                    let id = self.define_node(&fqn, db::node::GLOBAL_VARIABLE)?;
                    self.db.edge(db::edge::MEMBER, parent, id)?;
                    self.at(id, s.ident.span(), db::loc::TOKEN)?;
                    self.at(id, s.span(), db::loc::SCOPE)?;
                    self.emit_type(&s.ty, id, scope)?;
                }
                syn::Item::Type(t) => {
                    let fqn = join(scope, &t.ident.to_string());
                    let id = self.define_node(&fqn, db::node::TYPEDEF)?;
                    self.db.edge(db::edge::MEMBER, parent, id)?;
                    self.at(id, t.ident.span(), db::loc::TOKEN)?;
                    self.at(id, t.span(), db::loc::SCOPE)?;
                    self.emit_type(&t.ty, id, scope)?;
                }
                syn::Item::Macro(m) => {
                    if let Some(name) = &m.ident {
                        let fqn = join(scope, &name.to_string());
                        let id = self.define_node(&fqn, db::node::MACRO)?;
                        self.db.edge(db::edge::MEMBER, parent, id)?;
                        self.at(id, name.span(), db::loc::TOKEN)?;
                        self.at(id, m.span(), db::loc::SCOPE)?;
                    }
                }
                _ => {}
            }
        }
        Ok(())
    }

    fn emit_impl(&mut self, i: &syn::ItemImpl, scope: &mut Vec<String>, parent: i64) -> Result<()> {
        let Some(owner) = self.r.self_ty_fqn(&i.self_ty, scope) else { return Ok(()) };
        let owner_kind = self.index.is_defined(&owner).unwrap_or(db::node::STRUCT);
        let owner_id = self.node_for(&owner, owner_kind)?;
        self.at(owner_id, i.self_ty.span(), db::loc::TOKEN)?;

        // `impl Trait for Type` reads as Type inheriting Trait in the graph.
        let trait_fqn = i.trait_.as_ref().map(|(_, p, _)| self.r.resolve(p, self.index, scope, db::node::TYPE));
        if let (Some(tf), Some((_, p, _))) = (trait_fqn.as_ref(), i.trait_.as_ref()) {
            let tid = self.node_for(tf, db::node::INTERFACE)?;
            self.db.edge(db::edge::INHERITANCE, owner_id, tid)?;
            self.at(tid, p.span(), db::loc::TOKEN)?;
        }

        let mut inner = scope.clone();
        inner.push(owner.rsplit("::").next().unwrap_or(&owner).to_string());

        for item in &i.items {
            match item {
                syn::ImplItem::Fn(f) => {
                    let mfqn = join(&[owner.clone()], &f.sig.ident.to_string());
                    let mid = self.emit_signature(&f.sig, &mfqn, db::node::METHOD, &inner)?;
                    self.db.edge(db::edge::MEMBER, owner_id, mid)?;
                    self.at(mid, f.span(), db::loc::SCOPE)?;
                    if let Some(tf) = &trait_fqn {
                        let base = self.node_for(
                            &format!("{tf}::{}", f.sig.ident),
                            db::node::METHOD,
                        )?;
                        self.db.edge(db::edge::OVERRIDE, mid, base)?;
                    }
                    self.emit_body(mid, &f.sig, Some(&f.block), &inner)?;
                }
                syn::ImplItem::Const(c) => {
                    let cf = join(&[owner.clone()], &c.ident.to_string());
                    let id = self.define_node(&cf, db::node::GLOBAL_VARIABLE)?;
                    self.db.edge(db::edge::MEMBER, owner_id, id)?;
                    self.at(id, c.ident.span(), db::loc::TOKEN)?;
                    self.at(id, c.span(), db::loc::SCOPE)?;
                }
                syn::ImplItem::Type(t) => {
                    let tf = join(&[owner.clone()], &t.ident.to_string());
                    let id = self.define_node(&tf, db::node::TYPEDEF)?;
                    self.db.edge(db::edge::MEMBER, owner_id, id)?;
                    self.at(id, t.ident.span(), db::loc::TOKEN)?;
                    self.at(id, t.span(), db::loc::SCOPE)?;
                    self.emit_type(&t.ty, id, &inner)?;
                }
                _ => {}
            }
        }
        let _ = parent;
        Ok(())
    }

    fn emit_fields(
        &mut self,
        fields: &syn::Fields,
        owner: &str,
        owner_id: i64,
        scope: &[String],
    ) -> Result<()> {
        for (idx, f) in fields.iter().enumerate() {
            let name = f.ident.as_ref().map(|i| i.to_string()).unwrap_or_else(|| idx.to_string());
            let ffqn = join(&[owner.to_string()], &name);
            let id = self.define_node(&ffqn, db::node::FIELD)?;
            self.db.edge(db::edge::MEMBER, owner_id, id)?;
            match &f.ident {
                Some(i) => self.at(id, i.span(), db::loc::TOKEN)?,
                None => self.at(id, f.ty.span(), db::loc::TOKEN)?,
            }
            // SCOPE marks "defined here"; without it a leaf symbol's definition is
            // indistinguishable from its uses, which are TOKEN locations too.
            self.at(id, f.span(), db::loc::SCOPE)?;
            self.emit_type(&f.ty, id, scope)?;
        }
        Ok(())
    }

    fn emit_signature(
        &mut self,
        sig: &syn::Signature,
        fqn: &str,
        kind: i32,
        scope: &[String],
    ) -> Result<i64> {
        let id = self.define_node(fqn, kind)?;
        self.at(id, sig.ident.span(), db::loc::TOKEN)?;
        self.at(id, sig.span(), db::loc::SIGNATURE)?;

        for input in &sig.inputs {
            if let syn::FnArg::Typed(t) = input {
                self.emit_type(&t.ty, id, scope)?;
            }
        }
        if let syn::ReturnType::Type(_, t) = &sig.output {
            self.emit_type(t, id, scope)?;
        }
        Ok(id)
    }

    /// Every named type inside `ty` becomes a type-usage edge from `from`.
    fn emit_type(&mut self, ty: &syn::Type, from: i64, scope: &[String]) -> Result<()> {
        let mut paths = Vec::new();
        collect_type_paths(ty, &mut paths);
        for p in paths {
            let fqn = self.r.resolve(p, self.index, scope, db::node::TYPE);
            let id = self.node_for(&fqn, db::node::TYPE)?;
            if id != 0 && id != from {
                self.db.edge(db::edge::TYPE_USAGE, from, id)?;
                if let Some(last) = p.segments.last() {
                    self.at(id, last.ident.span(), db::loc::TOKEN)?;
                }
            }
        }
        Ok(())
    }

    fn emit_body(
        &mut self,
        owner: i64,
        sig: &syn::Signature,
        block: Option<&syn::Block>,
        scope: &[String],
    ) -> Result<()> {
        let Some(block) = block else { return Ok(()) };
        let mut body = Body { e: self, owner, scope: scope.to_vec(), locals: HashMap::new(), err: None };
        for input in &sig.inputs {
            match input {
                syn::FnArg::Receiver(r) => body.bind("self", r.self_token.span)?,
                syn::FnArg::Typed(t) => body.bind_pat(&t.pat)?,
            }
        }
        syn::visit::Visit::visit_block(&mut body, block);
        match body.err.take() {
            Some(e) => Err(e),
            None => Ok(()),
        }
    }
}

/// Walks a function body for calls, usages and locals.
struct Body<'a, 'b> {
    e: &'b mut Emitter<'a>,
    owner: i64,
    scope: Vec<String>,
    locals: HashMap<String, i64>,
    err: Option<anyhow::Error>,
}

impl Body<'_, '_> {
    fn take<T>(&mut self, r: Result<T>) -> Option<T> {
        match r {
            Ok(v) => Some(v),
            Err(e) => {
                self.err.get_or_insert(e);
                None
            }
        }
    }

    fn bind(&mut self, name: &str, span: proc_macro2::Span) -> Result<()> {
        // Local symbols are per function, so key them by the owning node.
        let key = format!("{}#{}", self.owner, name);
        let id = self.e.db.local(&key)?;
        self.locals.insert(name.to_string(), id);
        let file_id = self.e.file_id;
        self.e.db.location(id, file_id, range_of(span), db::loc::LOCAL_SYMBOL)
    }

    fn bind_pat(&mut self, pat: &syn::Pat) -> Result<()> {
        syn::visit::Visit::visit_pat(self, pat);
        match self.err.take() {
            Some(e) => Err(e),
            None => Ok(()),
        }
    }

    fn reference(&mut self, path: &syn::Path, kind: i32, fallback: i32) {
        let fqn = self.e.r.resolve(path, self.e.index, &self.scope, fallback);
        if fqn.is_empty() {
            return;
        }
        let n = self.e.node_for(&fqn, fallback);
        let Some(id) = self.take(n) else { return };
        if id == 0 {
            return;
        }
        let owner = self.owner;
        let r = self.e.db.edge(kind, owner, id);
        self.take(r);
        if let Some(last) = path.segments.last() {
            let r = self.e.at(id, last.ident.span(), db::loc::TOKEN);
            self.take(r);
        }
    }
}

impl<'ast> syn::visit::Visit<'ast> for Body<'_, '_> {
    fn visit_pat_ident(&mut self, node: &'ast syn::PatIdent) {
        let r = self.bind(&node.ident.to_string(), node.ident.span());
        self.take(r);
        syn::visit::visit_pat_ident(self, node);
    }

    fn visit_expr_call(&mut self, node: &'ast syn::ExprCall) {
        if let syn::Expr::Path(p) = &*node.func {
            self.reference(&p.path, db::edge::CALL, db::node::FUNCTION);
        } else {
            self.visit_expr(&node.func);
        }
        for a in &node.args {
            self.visit_expr(a);
        }
    }

    fn visit_expr_method_call(&mut self, node: &'ast syn::ExprMethodCall) {
        // ponytail: method calls resolve only when the name is unique crate-wide;
        // ambiguous ones are dropped rather than guessed. Upgrade path is
        // rust-analyzer's HIR, which knows receiver types.
        let name = node.method.to_string();
        let target = match self.e.index.methods.get(&name) {
            Some(c) if c.len() == 1 => Some(c[0].clone()),
            _ => None,
        };
        if let Some(fqn) = target {
            let n = self.e.node_for(&fqn, db::node::METHOD);
            if let Some(id) = self.take(n) {
                let owner = self.owner;
                let r = self.e.db.edge(db::edge::CALL, owner, id);
                self.take(r);
                let r = self.e.at(id, node.method.span(), db::loc::TOKEN);
                self.take(r);
            }
        }
        self.visit_expr(&node.receiver);
        for a in &node.args {
            self.visit_expr(a);
        }
    }

    fn visit_expr_path(&mut self, node: &'ast syn::ExprPath) {
        if node.path.segments.len() == 1 {
            let seg = &node.path.segments[0];
            let name = seg.ident.to_string();
            if let Some(&id) = self.locals.get(&name) {
                let r = self.e.at(id, seg.ident.span(), db::loc::LOCAL_SYMBOL);
                self.take(r);
                return;
            }
        }
        self.reference(&node.path, db::edge::USAGE, db::node::SYMBOL);
    }

    fn visit_expr_struct(&mut self, node: &'ast syn::ExprStruct) {
        self.reference(&node.path, db::edge::TYPE_USAGE, db::node::STRUCT);
        let base = self.e.r.resolve(&node.path, self.e.index, &self.scope, db::node::METHOD);
        for f in &node.fields {
            if let syn::Member::Named(id) = &f.member {
                let fqn = format!("{base}::{id}");
                if self.e.index.is_defined(&fqn).is_some() {
                    let n = self.e.node_for(&fqn, db::node::FIELD);
                    if let Some(nid) = self.take(n) {
                        let owner = self.owner;
                        let r = self.e.db.edge(db::edge::USAGE, owner, nid);
                        self.take(r);
                        let r = self.e.at(nid, id.span(), db::loc::TOKEN);
                        self.take(r);
                    }
                }
            }
            self.visit_expr(&f.expr);
        }
        if let Some(rest) = &node.rest {
            self.visit_expr(rest);
        }
    }

    fn visit_type_path(&mut self, node: &'ast syn::TypePath) {
        self.reference(&node.path, db::edge::TYPE_USAGE, db::node::TYPE);
        syn::visit::visit_type_path(self, node);
    }

    fn visit_macro(&mut self, node: &'ast syn::Macro) {
        // ponytail: macro bodies stay unparsed, so calls inside `println!` or
        // `tauri::generate_handler![]` are invisible. Parsing selected macros
        // is the upgrade if that turns out to matter.
        self.reference(&node.path, db::edge::MACRO_USAGE, db::node::MACRO);
    }
}

fn collect_use_paths(tree: &syn::UseTree, prefix: &mut Vec<String>, out: &mut Vec<String>) {
    match tree {
        syn::UseTree::Path(p) => {
            prefix.push(p.ident.to_string());
            collect_use_paths(&p.tree, prefix, out);
            prefix.pop();
        }
        syn::UseTree::Name(n) => out.push(join(prefix, &n.ident.to_string())),
        syn::UseTree::Rename(r) => out.push(join(prefix, &r.ident.to_string())),
        syn::UseTree::Glob(_) => {}
        syn::UseTree::Group(g) => {
            for t in &g.items {
                collect_use_paths(t, prefix, out);
            }
        }
    }
}

/// Named types nested anywhere inside a type, so `Vec<Result<Asset, Error>>`
/// yields Vec, Result, Asset and Error.
fn collect_type_paths<'t>(ty: &'t syn::Type, out: &mut Vec<&'t syn::Path>) {
    struct V<'t, 'o>(&'o mut Vec<&'t syn::Path>);
    impl<'t> syn::visit::Visit<'t> for V<'t, '_> {
        fn visit_type_path(&mut self, node: &'t syn::TypePath) {
            self.0.push(&node.path);
            syn::visit::visit_type_path(self, node);
        }
    }
    syn::visit::Visit::visit_type(&mut V(out), ty);
}
