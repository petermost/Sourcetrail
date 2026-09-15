# Rust indexer for Sourcetrail

Indexes Rust sources into Sourcetrail's SQLite format, so a Rust crate can be
browsed in the same graph/code view as the C++ and Java language packages.

Unlike those, this indexer is a separate executable rather than a compiled-in
language package. Sourcetrail already knows how to drive external indexers
through its **Custom Command** source group, which is all this needs — no
changes to the Sourcetrail C++ sources.

## Build

```
cd rust_indexer
cargo build --release
```

The binary lands in `target/release/sourcetrail_rust_indexer`.

## Use from Sourcetrail

New Project → add a source group of type **Custom Command**:

| Field             | Value                                                  |
|-------------------|--------------------------------------------------------|
| Source paths      | the crate's `src` directory                            |
| Source extensions | `.rs`                                                  |
| Custom command    | see below                                              |

```
/path/to/sourcetrail_rust_indexer --database-file-path %{DATABASE_FILE_PATH} --database-version %{DATABASE_VERSION} --source-file-path %{SOURCE_FILE_PATH}
```

Sourcetrail calls this once per `.rs` file and merges the results, so the
Refresh button re-indexes normally.

## Use standalone

Indexes a whole crate and writes a matching project file:

```
sourcetrail_rust_indexer \
    --crate-root path/to/crate \
    --database-file-path Project.srctrldb \
    --write-project Project.srctrlprj
```

## What it records

| Rust                          | Sourcetrail                        |
|-------------------------------|------------------------------------|
| module (file or `mod`)        | module, with member edges          |
| `struct` / `union` / field    | struct / union / field             |
| `enum` / variant              | enum / enum constant               |
| `trait` / supertrait          | interface / inheritance            |
| `impl Trait for Type`         | inheritance, methods get overrides |
| `fn` / method                 | function / method, with signature  |
| `const` / `static`            | global variable                    |
| `type` alias                  | typedef                            |
| `macro_rules!` and invocations| macro / macro usage                |
| `use`                         | import                             |
| calls, type usages            | call / type-usage edges            |
| parameters and `let` bindings | local symbols                      |

## Resolution, and where it stops

Parsing is `syn`, which is syntax only — there is no type inference. Names are
resolved from each file's `use` declarations, the enclosing module path and a
crate-wide symbol table built before emitting. That covers path calls,
types, fields and imports.

Two known ceilings, both marked `ponytail:` in `src/index.rs`:

* **Method calls** resolve only when the method name is unique across the
  crate. Ambiguous names are dropped rather than guessed at.
* **Macro bodies** are not parsed, so calls inside `println!` or
  `tauri::generate_handler![]` are invisible.

Lifting either one means switching the front end to rust-analyzer's HIR, which
knows receiver types and expands macros. Worth it only if the gaps show up in
practice.

## Storage version

The database layout is Sourcetrail's storage version **25**
(`src/lib/data/storage/sqlite/SqliteIndexStorage.cpp`). The indexer refuses to
run when Sourcetrail passes a different `%{DATABASE_VERSION}`, rather than
writing a database that would be silently misread.

## Sourcetrail selbst bauen (Arch, ohne C++/Java-Indexer)

Die mitgelieferten Presets erzwingen `BUILD_CXX_LANGUAGE_PACKAGE=ON`, was Clang 21.1
verlangt. Für Rust wird davon nichts gebraucht:

```bash
cmake --preset system-release \
  -DBUILD_CXX_LANGUAGE_PACKAGE=OFF \
  -DBUILD_JAVA_LANGUAGE_PACKAGE=OFF \
  -DBUILD_UNIT_TESTS_PACKAGE=OFF
cmake --build ../../build/system-release -j$(nproc)
```

Systempakete: `boost tinyxml qt6-base qt6-svg sqlite`. Boost-Header und
`boost-libs` müssen dieselbe Version haben (sonst fehlt `libboost_filesystem.so.<ver>`).
