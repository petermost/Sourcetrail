# Rust indexer for Sourcetrail

> Python lives next door in `../python_indexer`, TypeScript in `../ts_indexer`,
> and all three write into the **same** database — Sourcetrail merges source
> groups, so the whole repo ends up in one graph.

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

## Tauri commands, for the TypeScript indexer

`--dump-commands` prints every `#[tauri::command]` function as JSON, mapping the
name the frontend passes to `invoke(...)` onto the serialized node name of the
Rust function:

```
sourcetrail_rust_indexer --dump-commands --crate-root path/to/crate
```

No database is touched. The TypeScript indexer calls this to turn
`invoke('add_folder')` into a real call edge; asking here rather than
rebuilding the name over there keeps this indexer the only thing that decides
how a Rust symbol is named.

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

## MCP-Server

`mcp_server.py` stellt den fertigen Index als MCP-Werkzeuge bereit (stdio,
JSON-RPC, nur Python-stdlib — keine Abhängigkeiten):

| Werkzeug | Zweck |
|---|---|
| `search_symbols` | Symbole per Namensfragment finden, optional nach Art gefiltert |
| `symbol` | Ein Symbol komplett: Art, Signatur, Definitionsort, was es aufruft/nutzt, wer es aufruft/nutzt — je mit Fundstellen |
| `file_symbols` | Alle Definitionen einer Datei mit Zeilenbereichen |
| `reindex` | Index von Grund auf neu bauen (Rust, Python **und** TypeScript) |

Anbinden:

```bash
claude mcp add --scope user asset-bridge-index -- \
  python3 /home/elisha/surcetai/Sourcetrail/rust_indexer/mcp_server.py \
  --db /home/elisha/surcetai/asset-bridge-index/AssetBridge.srctrldb \
  --crate-root "/home/elisha/Dokumente/Asset Bridge/src-tauri"
```

`--python-root`/`--python-indexer` und `--typescript-root`/`--typescript-indexer`
lassen sich setzen; ohne Angabe ist die Wurzel jeweils der Ordner über dem Crate
und der Indexer der in `../python_indexer` bzw. `../ts_indexer`.

Selbsttest (Namensdekodierung + kompletter MCP-Handshake):

```bash
python3 mcp_server.py --db INDEX.srctrldb --crate-root DIR --self-test
```

### Bekannte Decke

Der Indexer hängt Quellpositionen an Knoten, nicht an Kanten. Der MCP-Server
leitet Fundstellen deshalb ab: ein Treffer ist ein Token des benutzten Symbols
innerhalb des Gültigkeitsbereichs des benutzenden. Das ist genau, solange ein
Symbol pro Zeile einmal vorkommt. Positionen an Kanten zu hängen wäre der
Ausbau — anzufassen erst, wenn die Ableitung tatsächlich stört.
