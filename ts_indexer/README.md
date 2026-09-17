# TypeScript indexer for Sourcetrail

> Rust lives in `../rust_indexer`, Python in `../python_indexer`, and all three
> write into the **same** database — Sourcetrail merges source groups, so the
> frontend, the backend and the build scripts share one graph.

Two halves:

* `ts_facts.mjs` — Node front end. Parses the tree with the TypeScript compiler
  API and prints facts as JSON. Touches no database.
* `sourcetrail_ts_indexer.py` — the writer. Reuses `Db` and the name packing
  from the Python indexer, so the SQLite schema has exactly one owner.

`typescript` comes from the **indexed project**, not from here — it is already a
devDependency of anything worth indexing. Resolution is tried from this file,
the project root and the working directory (a git worktree inherits its parent's
`node_modules`).

## Use from Sourcetrail

New Project → source group of type **Custom Command**, extensions `.ts` `.tsx`:

```
python3 /path/to/sourcetrail_ts_indexer.py --database-file-path "%{DATABASE_FILE_PATH}" --database-version %{DATABASE_VERSION} --source-file-path "%{SOURCE_FILE_PATH}" --project-root "/path/to/repo" --exclude .claude/worktrees
```

**The double quotes are not optional.** Sourcetrail splits the whole command on
whitespace (Boost.Process v1), so a path containing a space arrives as two
arguments without them.

## Use standalone

```
python3 sourcetrail_ts_indexer.py --project-root path/to/repo \
    --database-file-path Project.srctrldb --write-project Project.srctrlprj
```

## What it records

| TypeScript                        | Sourcetrail                     |
|-----------------------------------|---------------------------------|
| file                              | module, with member edges       |
| `class` / method / property       | class / method / field          |
| `interface` / its members         | interface / method / field      |
| `type` alias                      | typedef                         |
| `enum` / member                   | enum / enum constant            |
| `function`, `const f = () => {}`  | function, with signature        |
| other `const` / `let`             | global variable                 |
| `extends` / `implements`          | inheritance                     |
| relative `import`                 | import                          |
| calls, `<Component/>`, type refs  | call / type-usage edges         |

A destructured parameter keeps only its names in the signature: a React
component's props are an inline type literal with doc comments in it, and the
whole thing would be the symbol's name.

## Resolution, and where it stops

Parse only — `ts.createSourceFile`, no `ts.Program` and no `TypeChecker`, which
is what keeps a full pass over 117 files at ~2 s. A name resolves through the
enclosing class (and its written bases), the file's import bindings, the
enclosing module, or a project-wide unique name.

Four ceilings, all marked `ponytail:` in `ts_facts.mjs`:

* **Locals shadow.** Every parameter, binding and nested function name inside a
  definition is collected into one flat set and skipped. Without it a local
  `members` resolves to whatever unique top-level `members` the tree happens to
  hold — a wrong edge, which is worse than a missing one.
* **The unique-name fallback is for bare identifiers only.** `x.find(...)` on an
  unresolved `x` is an array method far more often than it is the project's one
  function called `find`.
* **Packages are not symbols.** Only relative specifiers resolve; `react`,
  `three` and `@tauri-apps/api` are outside the tree.
* **Nested arrows fold into the enclosing definition.** A callback's calls are
  recorded as the surrounding function's.

Lifting any of them means handing the front end a `ts.Program` with a
`TypeChecker`, which costs a full type-check per run. Worth it only if the gaps
show up in practice.

## The invoke() bridge

`invoke('add_folder')` is the only way the frontend reaches the Rust backend,
and it is a string, so nothing in the TypeScript tree links the two. The writer
closes that gap: it runs `sourcetrail_rust_indexer --dump-commands` over
`--tauri-crate` (default `<project-root>/src-tauri`), and every `invoke()` whose
literal matches a `#[tauri::command]` becomes a **call edge onto the existing
Rust node**, so a Custom Trail walks from a React component into the command and
on through the crate. A literal that matches nothing is dropped rather than
turned into a node of its own.

Missing crate or unbuilt Rust indexer is not an error — then there is simply
nothing to bridge, and the TypeScript index is written as before.

## Refresh cost

One file costs ~1.6 s, because the front end re-parses the whole tree to build
the symbol table before emitting. Sourcetrail's per-file Refresh over 117 files
is therefore minutes (the invoke() bridge rescans the crate each time, ~0.1 s
per file); the standalone run (and the MCP `reindex` tool) does all
of them in ~2.3 s. A facts cache would fix it and buys invalidation complexity
for a button nobody presses often.

## Self-check

```
cd /path/to/the/indexed/project        # needs a resolvable `typescript`
python3 /path/to/sourcetrail_ts_indexer.py --database-file-path /tmp/x.srctrldb --self-test
```

Builds a three-file temp tree and asserts the things that break when resolution
slips: a cross-file call, `new Store()` inside a `.tsx`, a class member edge, an
interface type usage, a same-file const — and that a second run over the same
database adds nothing.

## Storage version

Sourcetrail's storage version **25**. The indexer refuses to run when
Sourcetrail passes a different `%{DATABASE_VERSION}`, rather than writing a
database that would be silently misread.
