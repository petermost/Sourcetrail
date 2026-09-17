#!/usr/bin/env python3
"""Python indexer for Sourcetrail. Stdlib only - `ast` plus `sqlite3`.

Two ways to run it, same contract as the Rust indexer next door:

  * As a Sourcetrail "Custom Command" source group, once per source file:
      sourcetrail_python_indexer.py --database-file-path "%{DATABASE_FILE_PATH}" \
          --database-version %{DATABASE_VERSION} --source-file-path "%{SOURCE_FILE_PATH}"

  * Standalone over a whole tree:
      sourcetrail_python_indexer.py --project-root DIR --database-file-path out.srctrldb

The database layout is Sourcetrail's storage version 25, so this writes into the
same file as any other source group and the graph ends up shared.
"""

import argparse
import ast
import os
import sqlite3
import sys
import time

STORAGE_VERSION = 25

# src/lib/data/NodeKind.h
MODULE, CLASS, GLOBAL_VARIABLE = 1 << 3, 1 << 7, 1 << 10
FIELD, FUNCTION, METHOD, FILE = 1 << 11, 1 << 12, 1 << 13, 1 << 18
# src/lib/data/graph/Edge.h
MEMBER, TYPE_USAGE, CALL, INHERITANCE, IMPORT = 1 << 0, 1 << 1, 1 << 3, 1 << 4, 1 << 9
# src/lib/data/location/LocationType.h
TOKEN_LOC, SCOPE_LOC, ERROR_LOC = 0, 1, 6
DEF_EXPLICIT = 2

SCHEMA = """
CREATE TABLE IF NOT EXISTS meta(id INTEGER, key TEXT, value TEXT, PRIMARY KEY(id));
CREATE TABLE IF NOT EXISTS element(id INTEGER, PRIMARY KEY(id));
CREATE TABLE IF NOT EXISTS element_component(
    id INTEGER, element_id INTEGER, type INTEGER, data TEXT, PRIMARY KEY(id),
    FOREIGN KEY(element_id) REFERENCES element(id) ON DELETE CASCADE);
CREATE TABLE IF NOT EXISTS node(
    id INTEGER NOT NULL, type INTEGER NOT NULL, serialized_name TEXT, PRIMARY KEY(id),
    FOREIGN KEY(id) REFERENCES element(id) ON DELETE CASCADE);
CREATE TABLE IF NOT EXISTS edge(
    id INTEGER NOT NULL, type INTEGER NOT NULL, source_node_id INTEGER NOT NULL,
    target_node_id INTEGER NOT NULL, PRIMARY KEY(id),
    FOREIGN KEY(id) REFERENCES element(id) ON DELETE CASCADE,
    FOREIGN KEY(source_node_id) REFERENCES node(id) ON DELETE CASCADE,
    FOREIGN KEY(target_node_id) REFERENCES node(id) ON DELETE CASCADE);
CREATE TABLE IF NOT EXISTS symbol(
    id INTEGER NOT NULL, definition_kind INTEGER NOT NULL, PRIMARY KEY(id),
    FOREIGN KEY(id) REFERENCES node(id) ON DELETE CASCADE);
CREATE TABLE IF NOT EXISTS file(
    id INTEGER NOT NULL, path TEXT, language TEXT, modification_time TEXT,
    indexed INTEGER, complete INTEGER, line_count INTEGER, PRIMARY KEY(id),
    FOREIGN KEY(id) REFERENCES node(id) ON DELETE CASCADE);
CREATE TABLE IF NOT EXISTS filecontent(
    id INTEGER, content TEXT, PRIMARY KEY(id),
    FOREIGN KEY(id) REFERENCES file(id) ON DELETE CASCADE ON UPDATE CASCADE);
CREATE TABLE IF NOT EXISTS local_symbol(
    id INTEGER NOT NULL, name TEXT, PRIMARY KEY(id),
    FOREIGN KEY(id) REFERENCES element(id) ON DELETE CASCADE);
CREATE TABLE IF NOT EXISTS source_location(
    id INTEGER NOT NULL, file_node_id INTEGER, start_line INTEGER, start_column INTEGER,
    end_line INTEGER, end_column INTEGER, type INTEGER, PRIMARY KEY(id),
    FOREIGN KEY(file_node_id) REFERENCES node(id) ON DELETE CASCADE);
CREATE TABLE IF NOT EXISTS occurrence(
    element_id INTEGER NOT NULL, source_location_id INTEGER NOT NULL,
    PRIMARY KEY(element_id, source_location_id),
    FOREIGN KEY(element_id) REFERENCES element(id) ON DELETE CASCADE,
    FOREIGN KEY(source_location_id) REFERENCES source_location(id) ON DELETE CASCADE);
CREATE TABLE IF NOT EXISTS component_access(
    node_id INTEGER NOT NULL, type INTEGER NOT NULL, PRIMARY KEY(node_id),
    FOREIGN KEY(node_id) REFERENCES node(id) ON DELETE CASCADE);
CREATE TABLE IF NOT EXISTS error(
    id INTEGER NOT NULL, message TEXT, fatal INTEGER NOT NULL, indexed INTEGER NOT NULL,
    translation_unit TEXT, PRIMARY KEY(id),
    FOREIGN KEY(id) REFERENCES element(id) ON DELETE CASCADE);
CREATE INDEX IF NOT EXISTS node_serialized_name_idx ON node(serialized_name);
CREATE INDEX IF NOT EXISTS edge_source_target_idx ON edge(source_node_id, target_node_id);
"""


def serialize_name(parts, prefix="", postfix=""):
    """`NameHierarchy::serialize` - src/lib/data/name/NameHierarchy.cpp.

    Layout: `<delimiter>\tm<name>\ts<prefix>\tp<postfix>` joined by `\tn`.
    Only the last part carries prefix/postfix, which is where a signature goes.
    """
    out = "::\tm"
    for i, part in enumerate(parts):
        if i:
            out += "\tn"
        last = i + 1 == len(parts)
        out += part + "\ts" + (prefix if last else "") + "\tp" + (postfix if last else "")
    return out


def serialize_file_name(path):
    """Files use the `/` delimiter and one part holding the whole path."""
    return "/\tm" + path + "\ts\tp"


class Db:
    """Append-only writer for a .srctrldb. Mirrors rust_indexer/src/db.rs."""

    def __init__(self, path):
        self.c = sqlite3.connect(path, timeout=60)
        self.c.executescript(SCHEMA)
        if not self.c.execute("SELECT 1 FROM meta WHERE key='storage_version'").fetchone():
            self.c.execute("INSERT INTO meta(id, key, value) VALUES(NULL,'storage_version',?)",
                           (str(STORAGE_VERSION),))
        # Sourcetrail runs the command once per source file against the same
        # database, so these caches have to survive across processes or every
        # symbol shared by two files gets inserted twice.
        self.next_element = self.c.execute("SELECT IFNULL(MAX(id),0)+1 FROM element").fetchone()[0]
        self.next_loc = self.c.execute("SELECT IFNULL(MAX(id),0)+1 FROM source_location").fetchone()[0]
        self.nodes = {n: i for n, i in self.c.execute("SELECT serialized_name, id FROM node")}
        self.kinds = dict(self.c.execute("SELECT id, type FROM node"))
        self.edges = {(t, s, d): i for t, s, d, i
                      in self.c.execute("SELECT type, source_node_id, target_node_id, id FROM edge")}

    def _element(self):
        i = self.next_element
        self.next_element += 1
        self.c.execute("INSERT INTO element(id) VALUES(?)", (i,))
        return i

    def node(self, serialized, kind):
        """Insert, or return the existing node with the same serialized name.
        A more specific kind wins, so a forward reference gets upgraded later."""
        i = self.nodes.get(serialized)
        if i is not None:
            if kind not in (1 << 0, 1 << 1) and self.kinds.get(i) in (1 << 0, 1 << 1):
                self.c.execute("UPDATE node SET type=? WHERE id=?", (kind, i))
                self.kinds[i] = kind
            return i
        i = self._element()
        self.c.execute("INSERT INTO node(id, type, serialized_name) VALUES(?,?,?)",
                       (i, kind, serialized))
        self.nodes[serialized] = i
        self.kinds[i] = kind
        return i

    def define(self, node_id):
        self.c.execute("INSERT OR REPLACE INTO symbol(id, definition_kind) VALUES(?,?)",
                       (node_id, DEF_EXPLICIT))

    def file(self, path, content, language="python"):
        i = self.node(serialize_file_name(path), FILE)
        if not self.c.execute("SELECT 1 FROM file WHERE id=?", (i,)).fetchone():
            mtime = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(os.path.getmtime(path)))
            self.c.execute(
                "INSERT INTO file(id, path, language, modification_time, indexed, complete,"
                " line_count) VALUES(?,?,?,?,1,1,?)",
                (i, path, language, mtime, content.count("\n") + 1))
            self.c.execute("INSERT INTO filecontent(id, content) VALUES(?,?)", (i, content))
        return i

    def edge(self, kind, source, target):
        if not source or not target or source == target:
            return 0
        key = (kind, source, target)
        if key in self.edges:
            return self.edges[key]
        i = self._element()
        self.c.execute("INSERT INTO edge(id, type, source_node_id, target_node_id)"
                       " VALUES(?,?,?,?)", (i, kind, source, target))
        self.edges[key] = i
        return i

    def location(self, element_id, file_id, rng, kind):
        if not element_id:
            return
        i = self.next_loc
        self.next_loc += 1
        self.c.execute("INSERT INTO source_location(id, file_node_id, start_line, start_column,"
                       " end_line, end_column, type) VALUES(?,?,?,?,?,?,?)",
                       (i, file_id, rng[0], rng[1], rng[2], rng[3], kind))
        self.c.execute("INSERT OR IGNORE INTO occurrence(element_id, source_location_id)"
                       " VALUES(?,?)", (element_id, i))

    def error(self, message, file_path, rng):
        i = self._element()
        self.c.execute("INSERT INTO error(id, message, fatal, indexed, translation_unit)"
                       " VALUES(?,?,0,1,?)", (i, message, file_path))
        fid = self.nodes.get(serialize_file_name(file_path))
        if fid:
            self.location(i, fid, rng, ERROR_LOC)

    def set_project_settings(self, xml):
        self.c.execute("INSERT OR REPLACE INTO meta(id, key, value) VALUES("
                       "(SELECT id FROM meta WHERE key='project_settings'),'project_settings',?)",
                       (xml,))

    def counts(self):
        return (self.c.execute("SELECT COUNT(*) FROM node").fetchone()[0],
                self.c.execute("SELECT COUNT(*) FROM edge").fetchone()[0])

    def commit(self):
        self.c.commit()


# --- reading Python ------------------------------------------------------

def dotted(node):
    """`a.b.c` out of a Name/Attribute chain; None for anything else."""
    bits = []
    while isinstance(node, ast.Attribute):
        bits.append(node.attr)
        node = node.value
    if not isinstance(node, ast.Name):
        return None
    bits.append(node.id)
    return ".".join(reversed(bits))


def branches(n):
    """Every statement nested directly in a block statement, all arms included."""
    out = list(getattr(n, "body", [])) + list(getattr(n, "orelse", []))
    out += list(getattr(n, "finalbody", []))
    for h in getattr(n, "handlers", []):
        out += list(h.body)
    return out


def signature(fn):
    a = fn.args
    names = [p.arg for p in a.posonlyargs + a.args]
    if a.vararg:
        names.append("*" + a.vararg.arg)
    names += [p.arg for p in a.kwonlyargs]
    if a.kwarg:
        names.append("**" + a.kwarg.arg)
    ret = dotted(fn.returns) or "" if fn.returns else ""
    return "(" + ", ".join(names) + ")", ret


class Module:
    """One parsed source file: its definitions, its imports, its ranges."""

    def __init__(self, path, root):
        self.path = path
        self.src = open(path, encoding="utf-8", errors="replace").read()
        self.lines = self.src.splitlines()
        self.tree = ast.parse(self.src, filename=path)
        rel = os.path.relpath(path, root)
        self.parts = rel[:-3].split(os.sep)
        self.is_pkg = self.parts[-1] == "__init__"
        if self.is_pkg:
            self.parts.pop()
        self.qname = ".".join(self.parts)
        self.defs = {}      # qname -> (kind, ast node, signature or None)
        self.bases = {}     # class qname -> [base name as written]
        self.imports = {}   # local alias -> dotted target
        self._collect(self.tree.body, self.parts, None)

    def _pkg(self, level):
        base = list(self.parts) if self.is_pkg else self.parts[:-1]
        return base[:len(base) - (level - 1)] if level > 1 else base

    def _collect(self, stmts, parts, cls):
        for n in stmts:
            if isinstance(n, ast.ClassDef):
                q = parts + [n.name]
                self.defs[".".join(q)] = (CLASS, n, None)
                self.bases[".".join(q)] = [d for d in (dotted(b) for b in n.bases) if d]
                self._collect(n.body, q, ".".join(q))
            elif isinstance(n, (ast.FunctionDef, ast.AsyncFunctionDef)):
                q = parts + [n.name]
                self.defs[".".join(q)] = (METHOD if cls else FUNCTION, n, signature(n))
            elif isinstance(n, (ast.Assign, ast.AnnAssign)):
                kind = FIELD if cls else GLOBAL_VARIABLE
                targets = n.targets if isinstance(n, ast.Assign) else [n.target]
                for t in targets:
                    if isinstance(t, ast.Name):
                        self.defs[".".join(parts + [t.id])] = (kind, t, None)
            elif isinstance(n, ast.Import):
                for a in n.names:
                    self.imports[a.asname or a.name.split(".")[0]] = a.name
            elif isinstance(n, ast.ImportFrom):
                base = ".".join(self._pkg(n.level) + ([n.module] if n.module else [])) \
                    if n.level else (n.module or "")
                for a in n.names:
                    self.imports[a.asname or a.name] = (base + "." + a.name) if base else a.name
            elif isinstance(n, (ast.If, ast.Try, ast.With)):
                # conditional imports and platform-gated definitions are common
                self._collect(branches(n), parts, cls)

    def name_range(self, node, name):
        """Where the identifier itself sits - `ast` only gives the statement."""
        line = node.lineno
        text = self.lines[line - 1] if line <= len(self.lines) else ""
        col = text.find(name, node.col_offset)
        if col < 0:
            col = node.col_offset
        return (line, col + 1, line, col + len(name))

    def scope_range(self, node):
        return (node.lineno, node.col_offset + 1, node.end_lineno, node.end_col_offset)


class Project:
    """Every module under the root, plus the lookup that ties them together."""

    def __init__(self, root, excludes=()):
        self.root = root
        self.modules = {}
        self.errors = []
        for path in sorted(sources(root, excludes)):
            try:
                self.modules[path] = Module(path, root)
            except SyntaxError as e:
                self.errors.append((path, str(e), (e.lineno or 1, e.offset or 1,
                                                   e.lineno or 1, (e.offset or 1) + 1)))
        self.all = {}
        self.leaf = {}
        for m in self.modules.values():
            self.all[m.qname] = (MODULE, m)
            for q, (kind, _, _) in m.defs.items():
                self.all[q] = (kind, m)
                self.leaf.setdefault(q.split(".")[-1], set()).add(q)

    def by_suffix(self, name):
        if name in self.all:
            return name
        hits = [q for q in self.all if q.endswith("." + name)]
        return hits[0] if len(hits) == 1 else None

    def lookup(self, name, mod, cls):
        """Resolve a written name to a qualified one, or None.

        ponytail: syntax only, no type inference - an attribute call resolves
        through the enclosing class, an imported module, or a project-wide
        unique name. Ambiguous names are dropped rather than guessed at.
        Lifting this means a real type checker as the front end.
        """
        head, _, rest = name.partition(".")
        if cls:
            q = cls + "." + name
            if q in self.all:
                return q
            for base in self.bases_of(cls):
                q = base + "." + name
                if q in self.all:
                    return q
        q = ".".join(mod.parts + name.split("."))
        if q in self.all:
            return q
        target = mod.imports.get(head)
        if target:
            hit = self.by_suffix(target + ("." + rest if rest else ""))
            if hit:
                return hit
        cands = self.leaf.get(name.split(".")[-1])
        if cands and len(cands) == 1:
            return next(iter(cands))
        return None

    def bases_of(self, cls_qname):
        mod = self.all.get(cls_qname, (None, None))[1]
        if not mod:
            return []
        out = []
        for raw in mod.bases.get(cls_qname, []):
            hit = self.lookup(raw, mod, None)
            if hit:
                out.append(hit)
        return out


SKIP_DIRS = {".git", "node_modules", "__pycache__", ".venv", "venv", "target",
             "dist", "build", ".claude", "python-runtime", ".mypy_cache"}


def sources(root, excludes=()):
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames
                       if d not in SKIP_DIRS and not any(e in os.path.join(dirpath, d)
                                                         for e in excludes)]
        for f in filenames:
            if f.endswith(".py"):
                yield os.path.join(dirpath, f)


# --- writing one file into the index -------------------------------------

class Emitter(ast.NodeVisitor):
    """Second pass: walk one module and write its nodes, edges and locations."""

    def __init__(self, db, project, mod):
        self.db, self.project, self.mod = db, project, mod
        self.file_id = db.file(mod.path, mod.src)
        self.stack = []   # (node id, qname parts, enclosing class qname or None)

    def node_for(self, qname, kind, sig=None):
        prefix, postfix = "", ""
        if sig:
            postfix, prefix = sig
        return self.db.node(serialize_name(qname.split("."), prefix, postfix), kind)

    def run(self):
        mid = self.node_for(self.mod.qname, MODULE)
        self.db.define(mid)
        last = len(self.mod.lines) or 1
        self.db.location(mid, self.file_id, (1, 1, last, len(self.mod.lines[-1]) if self.mod.lines else 1),
                         SCOPE_LOC)
        self.stack.append((mid, self.mod.parts, None))
        self.emit_imports(mid)
        self.body(self.mod.tree.body, self.mod.parts, None)

    def emit_imports(self, mid):
        for alias, target in self.mod.imports.items():
            hit = self.by_target(target)
            if hit:
                self.db.edge(IMPORT, mid, hit)

    def by_target(self, target):
        q = self.project.by_suffix(target)
        if not q:
            return 0
        kind = self.project.all[q][0]
        return self.node_for(q, kind, self.sig_of(q))

    def sig_of(self, qname):
        kind, mod = self.project.all.get(qname, (None, None))
        if mod and qname in mod.defs:
            return mod.defs[qname][2]
        return None

    # -- definitions --
    def body(self, stmts, parts, cls):
        for n in stmts:
            if isinstance(n, ast.ClassDef):
                self.definition(n, parts, cls, CLASS, n.name)
            elif isinstance(n, (ast.FunctionDef, ast.AsyncFunctionDef)):
                self.definition(n, parts, cls, METHOD if cls else FUNCTION, n.name)
            elif isinstance(n, (ast.Assign, ast.AnnAssign)):
                targets = n.targets if isinstance(n, ast.Assign) else [n.target]
                for t in targets:
                    if isinstance(t, ast.Name):
                        q = ".".join(parts + [t.id])
                        nid = self.node_for(q, FIELD if cls else GLOBAL_VARIABLE)
                        self.db.define(nid)
                        self.db.edge(MEMBER, self.stack[-1][0], nid)
                        self.db.location(nid, self.file_id, self.mod.name_range(t, t.id), TOKEN_LOC)
                        self.db.location(nid, self.file_id, self.mod.scope_range(n), SCOPE_LOC)
                if getattr(n, "value", None) is not None:
                    self.references(n.value, cls)
            elif isinstance(n, (ast.Import, ast.ImportFrom)):
                pass  # handled once, at module level
            elif isinstance(n, (ast.If, ast.Try, ast.With, ast.For, ast.While)):
                self.body(branches(n), parts, cls)
                self.references(n, cls, skip_body=True)
            else:
                self.references(n, cls)

    def definition(self, n, parts, cls, kind, name):
        q = ".".join(parts + [name])
        sig = self.mod.defs.get(q, (None, None, None))[2]
        nid = self.node_for(q, kind, sig)
        self.db.define(nid)
        self.db.edge(MEMBER, self.stack[-1][0], nid)
        self.db.location(nid, self.file_id, self.mod.name_range(n, name), TOKEN_LOC)
        self.db.location(nid, self.file_id, self.mod.scope_range(n), SCOPE_LOC)
        if kind == CLASS:
            for base in n.bases:
                raw = dotted(base)
                target = raw and self.project.lookup(raw, self.mod, cls)
                if target:
                    tid = self.node_for(target, self.project.all[target][0])
                    self.db.edge(INHERITANCE, nid, tid)
                    self.db.location(tid, self.file_id, self.mod.name_range(base, raw.split(".")[-1]),
                                     TOKEN_LOC)
            self.stack.append((nid, parts + [name], q))
            self.body(n.body, parts + [name], q)
            self.stack.pop()
        else:
            self.stack.append((nid, parts + [name], cls))
            for stmt in n.body:
                self.references(stmt, cls)
            self.stack.pop()

    # -- references --
    def references(self, tree, cls, skip_body=False):
        """Everything a definition body reaches out to.

        ponytail: a nested `def` is not its own symbol - its calls land on the
        enclosing one. Give it a node when nested closures start mattering.
        """
        holder = self.stack[-1][0]
        walk = ast.iter_child_nodes(tree) if skip_body else [tree]
        for top in walk:
            if skip_body and isinstance(top, ast.stmt):
                continue
            for n in ast.walk(top):
                if isinstance(n, ast.Call):
                    self.reference(n.func, cls, holder, CALL)
                elif isinstance(n, ast.Name) and isinstance(n.ctx, ast.Load):
                    self.reference(n, cls, holder, TYPE_USAGE, quiet=True)

    def reference(self, expr, cls, holder, kind, quiet=False):
        raw = dotted(expr)
        if not raw:
            return
        target = self.project.lookup(raw, self.mod, cls)
        if not target:
            return
        tkind = self.project.all[target][0]
        if quiet and tkind not in (CLASS, GLOBAL_VARIABLE):
            return
        tid = self.node_for(target, tkind, self.sig_of(target))
        self.db.edge(kind if not quiet else TYPE_USAGE, holder, tid)
        leaf = raw.split(".")[-1]
        self.db.location(tid, self.file_id, self.mod.name_range(expr, leaf), TOKEN_LOC)


PROJECT_XML = """<?xml version="1.0" encoding="utf-8" ?>
<config>
    <source_groups>
        <source_group_3c8a51d2-7e40-4b19-ae62-0f9d4c15b7aa>
            <custom_command>{exe} --database-file-path "%{{DATABASE_FILE_PATH}}" --database-version %{{DATABASE_VERSION}} --source-file-path "%{{SOURCE_FILE_PATH}}" --project-root "{root}"</custom_command>
            <name>Python</name>
            <run_in_parallel>0</run_in_parallel>
            <source_extensions>
                <source_extension>.py</source_extension>
            </source_extensions>
            <source_paths>
                <source_path>{root}</source_path>
            </source_paths>
            <status>enabled</status>
            <type>Custom Command Source Group</type>
        </source_group_3c8a51d2-7e40-4b19-ae62-0f9d4c15b7aa>
    </source_groups>
    <version>8</version>
</config>
"""


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--database-file-path", required=True)
    p.add_argument("--source-file-path", action="append", default=[],
                   help="index only this file (repeatable); default: the whole tree")
    p.add_argument("--project-root", default=None, help="directory the module paths are relative to")
    p.add_argument("--database-version", type=int, default=None)
    p.add_argument("--exclude", action="append", default=[], help="substring of paths to skip")
    p.add_argument("--project-file", default=None,
                   help="store this .srctrlprj in the database, so Sourcetrail does not "
                        "call the index outdated on every open")
    p.add_argument("--write-project", default=None)
    a = p.parse_args(argv)

    if a.database_version is not None and a.database_version != STORAGE_VERSION:
        sys.exit(f"Sourcetrail expects storage version {a.database_version}, this indexer writes "
                 f"{STORAGE_VERSION}.")

    root = a.project_root
    if not root:
        start = a.source_file_path[0] if a.source_file_path else "."
        root = os.path.dirname(os.path.abspath(start))
    root = os.path.abspath(root)

    # ponytail: a per-file run re-parses the whole tree to resolve cross-file
    # names. 39 files is under a second; batch the source group if it grows.
    project = Project(root, a.exclude)
    targets = [os.path.abspath(s) for s in a.source_file_path] or list(project.modules)

    db = Db(a.database_file_path)
    done = 0
    for path in targets:
        mod = project.modules.get(path) or project.modules.get(os.path.realpath(path))
        if mod is None:
            continue
        Emitter(db, project, mod).run()
        done += 1
    for path, message, rng in project.errors:
        if path in targets:
            db.error(message, path, rng)

    xml = PROJECT_XML.format(exe=os.path.abspath(__file__), root=root)
    if a.write_project:
        db.set_project_settings(xml)
        open(a.write_project, "w").write(xml)
    if a.project_file:
        db.set_project_settings(open(a.project_file).read())
    db.commit()

    nodes, edges = db.counts()
    print(f"indexed {done} python file(s) under {root} -> {nodes} nodes, {edges} edges in "
          f"{a.database_file_path}")
    return 0


def self_test():
    """Round-trips the name packing and indexes a two-file tree from scratch."""
    import tempfile, textwrap
    assert serialize_name(["a", "b"], "int", "(x)") == "::\tma\ts\tp\tnb\tsint\tp(x)"
    assert serialize_file_name("/x.py") == "/\tm/x.py\ts\tp"

    with tempfile.TemporaryDirectory() as d:
        os.makedirs(os.path.join(d, "pkg"))
        open(os.path.join(d, "pkg", "__init__.py"), "w").write("")
        open(os.path.join(d, "pkg", "core.py"), "w").write(textwrap.dedent("""\
            LIMIT = 5
            class Base:
                def run(self): pass
            class Child(Base):
                def run(self):
                    return helper(LIMIT)
            def helper(n):
                return n
            """))
        open(os.path.join(d, "app.py"), "w").write(textwrap.dedent("""\
            from pkg.core import helper, Child
            def main():
                c = Child()
                return helper(1)
            """))
        db_path = os.path.join(d, "t.srctrldb")
        main(["--database-file-path", db_path, "--project-root", d])

        c = sqlite3.connect(db_path)
        names = {n: i for n, i in c.execute("SELECT serialized_name, id FROM node")}
        readable = {s.split("\tm", 1)[1].replace("\ts\tp\tn", ".").split("\ts")[0]: i
                    for s, i in names.items()}
        for want in ("pkg.core.helper", "pkg.core.Child", "app.main", "pkg.core.LIMIT"):
            assert want in readable, f"missing {want}; got {sorted(readable)}"
        edges = {(t, s, d_) for t, s, d_ in
                 c.execute("SELECT type, source_node_id, target_node_id FROM edge")}
        assert (CALL, readable["app.main"], readable["pkg.core.helper"]) in edges, \
            "cross-file call not resolved"
        assert (INHERITANCE, readable["pkg.core.Child"], readable["pkg.core.Base"]) in edges, \
            "base class not resolved"
        assert (CALL, readable["pkg.core.Child.run"], readable["pkg.core.helper"]) in edges, \
            "same-file call not resolved"
        assert any(t == MEMBER and s == readable["pkg.core.Child"] for t, s, _ in edges), \
            "class has no members"
        # a second run over the same database must not duplicate anything
        before = c.execute("SELECT COUNT(*) FROM node").fetchone()[0]
        main(["--database-file-path", db_path, "--project-root", d])
        after = sqlite3.connect(db_path).execute("SELECT COUNT(*) FROM node").fetchone()[0]
        assert before == after, f"re-run duplicated nodes: {before} -> {after}"
    print("self-test ok")


if __name__ == "__main__":
    if "--self-test" in sys.argv:
        self_test()
    else:
        sys.exit(main())
