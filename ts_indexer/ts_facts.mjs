#!/usr/bin/env node
// Front end of the Sourcetrail TypeScript indexer.
//
// Parses a tree with the TypeScript compiler API and prints the facts as JSON
// on stdout. The writing half is sourcetrail_ts_indexer.py, which reuses the
// Python indexer's Db class - the Sourcetrail schema has one owner, not two.
//
//   node ts_facts.mjs --project-root DIR [--source-file-path FILE]... [--exclude SUB]...

import fs from 'node:fs'
import path from 'node:path'
import { createRequire } from 'node:module'

const args = { root: '.', targets: [], excludes: [] }
for (let i = 2; i < process.argv.length; i += 2) {
  const [flag, value] = [process.argv[i], process.argv[i + 1]]
  if (flag === '--project-root') args.root = value
  else if (flag === '--source-file-path') args.targets.push(path.resolve(value))
  else if (flag === '--exclude') args.excludes.push(value)
  else { console.error(`unknown argument: ${flag}`); process.exit(2) }
}
const root = path.resolve(args.root)

// `typescript` is a devDependency of the indexed project, not of this indexer.
// Node resolves from the *importer*, so ask from the project root and the
// working directory as well - a worktree inherits node_modules from its parent.
let ts
for (const from of [import.meta.url, path.join(root, '_'), path.join(process.cwd(), '_')]) {
  try { ts = createRequire(from)('typescript'); break } catch { /* next */ }
}
if (!ts) {
  console.error(`no typescript package resolvable from ${root} or ${process.cwd()}`)
  process.exit(2)
}

const SKIP_DIRS = new Set(['.git', 'node_modules', 'dist', 'build', 'target',
  '.claude', '__pycache__', '.venv', 'python-runtime'])

function sources(dir, out = []) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, e.name)
    if (e.isDirectory()) {
      if (SKIP_DIRS.has(e.name) || args.excludes.some((x) => p.includes(x))) continue
      sources(p, out)
    } else if (/\.tsx?$/.test(e.name) && !e.name.endsWith('.d.ts')) out.push(p)
  }
  return out
}

/** `src/utils/channels.ts` -> `src.utils.channels`; `.../index.ts` drops the leaf. */
function moduleOf(file) {
  const parts = path.relative(root, file).replace(/\.tsx?$/, '').split(path.sep)
  if (parts[parts.length - 1] === 'index') parts.pop()
  return parts.join('.')
}

const files = new Map()   // absolute path -> { sf, mod }
for (const f of sources(root)) {
  const text = fs.readFileSync(f, 'utf8')
  files.set(f, {
    sf: ts.createSourceFile(f, text, ts.ScriptTarget.Latest, true,
      f.endsWith('.tsx') ? ts.ScriptKind.TSX : ts.ScriptKind.TS),
    mod: moduleOf(f),
  })
}

// --- ranges and names ----------------------------------------------------

/** 1-based line/column, end column inclusive - what Sourcetrail stores. */
function rangeOf(sf, node) {
  const s = sf.getLineAndCharacterOfPosition(node.getStart(sf))
  const e = sf.getLineAndCharacterOfPosition(node.getEnd())
  return [s.line + 1, s.character + 1, e.line + 1, e.character]
}

/**
 * The written signature, on one line.
 *
 * A destructured parameter keeps only its names: a React component's props are
 * an inline type literal with doc comments in it, and printing that whole thing
 * makes the symbol name several hundred characters of prose.
 */
function sigOf(sf, n) {
  if (!n || !n.parameters) return null
  const one = (t) => t.getText(sf).replace(/\/\*[\s\S]*?\*\//g, '').replace(/\s+/g, ' ').trim()
  const ps = n.parameters.map((p) =>
    ts.isObjectBindingPattern(p.name) || ts.isArrayBindingPattern(p.name)
      ? one(p.name) : one(p))
  return [`(${ps.join(', ')})`, n.type ? one(n.type) : '']
}

/** A dotted name plus the identifier its last part sits on, or null. */
function dotted(n) {
  if (!n) return null
  if (ts.isIdentifier(n)) return { raw: n.text, leaf: n }
  if (n.kind === ts.SyntaxKind.ThisKeyword) return { raw: 'this', leaf: n }
  if (ts.isPropertyAccessExpression(n) || ts.isQualifiedName(n)) {
    const name = ts.isPropertyAccessExpression(n) ? n.name : n.right
    if (!ts.isIdentifier(name)) return null
    const left = dotted(ts.isPropertyAccessExpression(n) ? n.expression : n.left)
    return { raw: left ? `${left.raw}.${name.text}` : name.text, leaf: name }
  }
  return null
}

// --- pass 1: every declaration in the tree -------------------------------

const all = new Map()     // qname -> { kind, sig }
const byLeaf = new Map()  // last name part -> Set<qname>
const bases = new Map()   // class qname -> [written base names]
const defaults = new Map()// module qname -> qname of its `export default` declaration

function record(q, kind, sig) {
  all.set(q, { kind, sig: sig || null })
  const leaf = q.slice(q.lastIndexOf('.') + 1)
  if (!byLeaf.has(leaf)) byLeaf.set(leaf, new Set())
  byLeaf.get(leaf).add(q)
}

function isDefault(n) {
  return (n.modifiers || []).some((m) => m.kind === ts.SyntaxKind.DefaultKeyword)
}

/** The shape of a value declaration: `const f = () => {}` is a function. */
function initKind(d) {
  const i = d.initializer
  return i && (ts.isArrowFunction(i) || ts.isFunctionExpression(i)) ? 'function' : 'globalvar'
}

function declare(sf, mod, stmt, emit) {
  const q = (name) => `${mod}.${name}`
  if (ts.isFunctionDeclaration(stmt) && stmt.name) {
    emit(q(stmt.name.text), 'function', sigOf(sf, stmt), stmt.name, stmt, mod)
    if (isDefault(stmt)) defaults.set(mod, q(stmt.name.text))
  } else if (ts.isClassDeclaration(stmt) && stmt.name) {
    const cq = q(stmt.name.text)
    emit(cq, 'class', null, stmt.name, stmt, mod)
    if (isDefault(stmt)) defaults.set(mod, cq)
    bases.set(cq, (stmt.heritageClauses || []).flatMap((h) =>
      h.types.map((t) => dotted(t.expression)).filter(Boolean)))
    for (const m of stmt.members) {
      if (!m.name || !ts.isIdentifier(m.name)) continue
      const kind = ts.isMethodDeclaration(m) || ts.isGetAccessor(m) || ts.isSetAccessor(m)
        ? 'method' : 'field'
      emit(`${cq}.${m.name.text}`, kind, sigOf(sf, m), m.name, m, cq)
    }
  } else if (ts.isInterfaceDeclaration(stmt)) {
    const iq = q(stmt.name.text)
    emit(iq, 'interface', null, stmt.name, stmt, mod)
    bases.set(iq, (stmt.heritageClauses || []).flatMap((h) =>
      h.types.map((t) => dotted(t.expression)).filter(Boolean)))
    for (const m of stmt.members) {
      if (!m.name || !ts.isIdentifier(m.name)) continue
      emit(`${iq}.${m.name.text}`, ts.isMethodSignature(m) ? 'method' : 'field',
        sigOf(sf, m), m.name, m, iq)
    }
  } else if (ts.isTypeAliasDeclaration(stmt)) {
    emit(q(stmt.name.text), 'typedef', null, stmt.name, stmt, mod)
  } else if (ts.isEnumDeclaration(stmt)) {
    const eq = q(stmt.name.text)
    emit(eq, 'enum', null, stmt.name, stmt, mod)
    for (const m of stmt.members) {
      if (ts.isIdentifier(m.name)) emit(`${eq}.${m.name.text}`, 'enumconst', null, m.name, m, eq)
    }
  } else if (ts.isVariableStatement(stmt)) {
    for (const d of stmt.declarationList.declarations) {
      if (!ts.isIdentifier(d.name)) continue   // destructuring is not a symbol
      emit(q(d.name.text), initKind(d), sigOf(sf, d.initializer), d.name, d, mod)
      if (isDefault(stmt)) defaults.set(mod, q(d.name.text))
    }
  }
}

for (const { sf, mod } of files.values()) {
  record(mod, 'module')
  for (const stmt of sf.statements) declare(sf, mod, stmt, (q, kind, sig) => record(q, kind, sig))
}

// --- imports and resolution ----------------------------------------------

/** Relative specifier -> module qname, or null for a package outside the tree. */
function resolveSpec(spec, from) {
  if (!spec.startsWith('.')) return null
  const base = path.resolve(path.dirname(from), spec)
  for (const c of [`${base}.ts`, `${base}.tsx`,
    path.join(base, 'index.ts'), path.join(base, 'index.tsx')]) {
    if (files.has(c)) return files.get(c).mod
  }
  return null
}

/** Local binding name -> qname it stands for. */
function importsOf(sf, file) {
  const out = new Map()
  const at = []
  for (const stmt of sf.statements) {
    if (!ts.isImportDeclaration(stmt) || !stmt.importClause) continue
    const mod = resolveSpec(stmt.moduleSpecifier.text, file)
    if (!mod) continue   // ponytail: packages are not symbols here; see README
    const c = stmt.importClause
    if (c.name) {
      const q = defaults.get(mod) || mod
      out.set(c.name.text, q)
      at.push({ to: q, at: rangeOf(sf, c.name) })
    }
    if (c.namedBindings && ts.isNamespaceImport(c.namedBindings)) {
      out.set(c.namedBindings.name.text, mod)
      at.push({ to: mod, at: rangeOf(sf, c.namedBindings.name) })
    } else if (c.namedBindings) {
      for (const e of c.namedBindings.elements) {
        const q = `${mod}.${(e.propertyName || e.name).text}`
        out.set(e.name.text, q)
        at.push({ to: q, at: rangeOf(sf, e.name) })
      }
    }
  }
  return { map: out, at }
}

/**
 * Resolve a written name to a qualified one, or null.
 *
 * ponytail: syntax only, no type checker - a member call resolves through the
 * enclosing class, an import binding, or a project-wide unique name. Ambiguous
 * names are dropped rather than guessed at. Lifting this means handing the
 * front end a ts.Program with a TypeChecker, which costs a full type-check per
 * run; worth it only if the gaps show up in practice.
 */
function lookup(raw, ctx) {
  const name = raw.startsWith('this.') ? raw.slice(5) : raw
  if (!name) return null
  const parts = name.split('.')
  const cands = []
  if (ctx.cls) {
    cands.push(`${ctx.cls}.${name}`)
    for (const b of bases.get(ctx.cls) || []) {
      const bq = lookup(b.raw, { mod: ctx.mod, imports: ctx.imports })
      if (bq) cands.push(`${bq}.${name}`)
    }
  }
  cands.push(`${ctx.mod}.${name}`)
  const bound = ctx.imports.get(parts[0])
  if (bound) cands.push([bound, ...parts.slice(1)].join('.'))
  for (const c of cands) if (all.has(c)) return c
  // The unique-name fallback is for bare identifiers only. `x.find(...)` on an
  // unresolved `x` is an array method far more often than it is the project's
  // one function called `find`, and guessing there filled the graph with edges
  // from every `.sort()`, `.set()` and `.find()` in the tree.
  if (parts.length > 1) return null
  const hits = byLeaf.get(parts[0])
  return hits && hits.size === 1 ? [...hits][0] : null
}

/**
 * Every name bound inside a definition: parameters, `const`/`let`, destructured
 * fields, nested functions, catch variables.
 *
 * ponytail: one flat set for the whole definition, so an inner binding shadows
 * the outer lines too. Without it a local `members` resolves to whatever unique
 * top-level `members` the tree happens to hold - a wrong edge, not a missing one.
 */
function localsOf(node) {
  const out = new Set()
  const add = (n) => {
    if (!n) return
    if (ts.isIdentifier(n)) out.add(n.text)
    else if (ts.isObjectBindingPattern(n) || ts.isArrayBindingPattern(n)) {
      for (const e of n.elements) if (ts.isBindingElement(e)) add(e.name)
    }
  }
  const visit = (n) => {
    if (ts.isParameter(n) || ts.isVariableDeclaration(n)) add(n.name)
    else if (ts.isCatchClause(n) && n.variableDeclaration) add(n.variableDeclaration.name)
    else if ((ts.isFunctionDeclaration(n) || ts.isClassDeclaration(n)) && n.name) add(n.name)
    n.forEachChild(visit)
  }
  visit(node)
  return out
}

// --- pass 2: the facts of one file ---------------------------------------

function factsOf(file) {
  const { sf, mod } = files.get(file)
  const { map, at } = importsOf(sf, file)
  const defs = []
  const refs = []
  const invokes = []
  const last = sf.getLineAndCharacterOfPosition(sf.getEnd())

  const ref = (holder, node, kind, ctx) => {
    const d = dotted(node)
    if (!d) return
    const head = (d.raw.startsWith('this.') ? d.raw.slice(5) : d.raw).split('.')[0]
    if (ctx.locals.has(head)) return
    const target = lookup(d.raw, ctx)
    if (!target || target === holder) return
    // A bare identifier is only worth an edge when it names something usable.
    if (kind === 'use' && !['class', 'interface', 'typedef', 'enum', 'globalvar', 'module']
      .includes(all.get(target).kind)) return
    refs.push({ from: holder, to: target, kind, at: rangeOf(sf, d.leaf) })
  }

  /** Everything a definition body reaches out to. Nested arrows land on it too. */
  const body = (node, holder, ctx) => {
    const visit = (n) => {
      if (ts.isCallExpression(n) || ts.isNewExpression(n)) {
        // invoke('add_folder') is the only way the frontend reaches the Rust
        // backend. The name is a literal, so the edge can be resolved without
        // type information; an unknown name is dropped by the writer.
        const arg = n.arguments && n.arguments[0]
        if (ts.isCallExpression(n) && ts.isIdentifier(n.expression) &&
            n.expression.text === 'invoke' && arg && ts.isStringLiteralLike(arg)) {
          invokes.push({ from: holder, name: arg.text, at: rangeOf(sf, arg) })
        }
        ref(holder, n.expression, 'call', ctx)
        n.forEachChild(visit)
      } else if (ts.isJsxOpeningElement(n) || ts.isJsxSelfClosingElement(n)) {
        // <ModalShell/> is how a React component gets called; <div/> is not one.
        if (/^[A-Z]/.test(n.tagName.getText(sf))) ref(holder, n.tagName, 'call', ctx)
        n.forEachChild(visit)
      } else if (ts.isTypeReferenceNode(n)) {
        ref(holder, n.typeName, 'type', ctx)
      } else if (ts.isPropertyAccessExpression(n)) {
        ref(holder, n, 'use', ctx)
        visit(n.expression)
      } else if (ts.isIdentifier(n)) {
        ref(holder, n, 'use', ctx)
      } else n.forEachChild(visit)
    }
    node.forEachChild(visit)
  }

  for (const stmt of sf.statements) {
    declare(sf, mod, stmt, (q, kind, sig, nameNode, scopeNode, parent) => {
      defs.push({ q, kind, sig, parent, name: rangeOf(sf, nameNode), scope: rangeOf(sf, scopeNode) })
      const cls = ['class', 'interface'].includes(kind) ? q : null
      const ctx = { mod, imports: map, locals: localsOf(scopeNode),
        cls: cls || (parent !== mod ? parent : null) }
      if (cls) {
        for (const b of bases.get(q) || []) {
          const target = lookup(b.raw, ctx)
          if (target) refs.push({ from: q, to: target, kind: 'inherit', at: rangeOf(sf, b.leaf) })
        }
      }
      body(scopeNode, q, ctx)
    })
  }
  return { path: file, mod, lastLine: last.line + 1, lastCol: last.character + 1,
    defs, refs, invokes, imports: at }
}

const targets = args.targets.length ? args.targets.filter((t) => files.has(t)) : [...files.keys()]
const symbols = {}
for (const [q, v] of all) symbols[q] = [v.kind, v.sig]
process.stdout.write(JSON.stringify({ root, symbols, files: targets.map(factsOf) }))
