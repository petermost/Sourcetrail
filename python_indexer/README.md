# Python indexer for Sourcetrail

Indexes Python sources into Sourcetrail's SQLite format. Same idea as
`../rust_indexer`, driven through the same **Custom Command** source group — and
because Sourcetrail merges several source groups into one database, Rust, Python
and TypeScript (`../ts_indexer`, which reuses the `Db` and name packing from
here) end up in **one** graph.

No build step and no dependencies: `ast` and `sqlite3` from the standard library.

## Use from Sourcetrail

New Project → source group of type **Custom Command**:

| Field             | Value                                |
|-------------------|--------------------------------------|
| Source paths      | the directories holding `.py` files  |
| Source extensions | `.py`                                |
| Custom command    | see below                            |

```
python3 /path/to/sourcetrail_python_indexer.py --database-file-path "%{DATABASE_FILE_PATH}" --database-version %{DATABASE_VERSION} --source-file-path "%{SOURCE_FILE_PATH}" --project-root "/path/to/repo"
```

**The path placeholders must stay double-quoted.** Sourcetrail hands the command
string to Boost.Process, which splits it on whitespace; an unquoted path with a
space in it arrives as two arguments and the indexer rejects the second one.

`--project-root` is what module paths are relative to — pass the repo root, so
`scripts/mtlxbuild/build.py` becomes `scripts.mtlxbuild.build`.

## Use standalone

```
sourcetrail_python_indexer.py --project-root path/to/repo \
    --database-file-path Project.srctrldb \
    --exclude .claude/worktrees
```

## What it records

| Python                        | Sourcetrail                    |
|-------------------------------|--------------------------------|
| module (one per file)         | module, with member edges      |
| `class`                       | class                          |
| base classes                  | inheritance                    |
| `def` at module level         | function, with signature       |
| `def` in a class              | method, with signature         |
| module-level assignment       | global variable                |
| class-level assignment        | field                          |
| `import` / `from … import`    | import                         |
| calls                         | call                           |
| classes and constants read    | type usage                     |

## Resolution, and where it stops

`ast` is syntax only — no type inference. A name resolves through the enclosing
class and its bases, the module's own scope, the file's imports, and finally a
project-wide unique leaf name. Ambiguous names are **dropped rather than
guessed at**.

Two ceilings, both marked `ponytail:` in the source:

* `obj.method()` resolves only when the method name is unique project-wide —
  there is nothing that knows what `obj` is.
* A nested `def` is not its own symbol; its calls land on the enclosing one.

Lifting either means a real type checker as the front end (jedi, pyright).
Worth it only when the gaps show up in practice.

A per-file run re-parses the whole tree to resolve cross-file names. At 41 files
that is half a second; batch the source group if the tree grows a lot.

## Self-check

```
python3 sourcetrail_python_indexer.py --self-test
```

Round-trips the name packing and indexes a two-file tree from scratch, asserting
the cross-file call, the base class, the member edges — and that a second run
over the same database duplicates nothing (Sourcetrail calls the command once
per source file against one database, so the id caches have to survive across
processes).

## Storage version

Sourcetrail storage version **25**. The indexer refuses to run when Sourcetrail
passes a different `%{DATABASE_VERSION}` rather than writing a database that
would be silently misread.
