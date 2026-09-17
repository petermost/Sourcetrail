#!/usr/bin/env python3
"""TypeScript indexer for Sourcetrail.

Two halves: `ts_facts.mjs` reads the tree with the TypeScript compiler API and
prints facts as JSON, this script writes them into the .srctrldb. The writer
itself is the Python indexer's `Db` - one owner for the Sourcetrail schema.

Two ways to run it:

  * As a Sourcetrail "Custom Command" source group, once per source file:
      sourcetrail_ts_indexer.py --database-file-path "%{DATABASE_FILE_PATH}" \
          --database-version %{DATABASE_VERSION} \
          --source-file-path "%{SOURCE_FILE_PATH}" --project-root "<repo>"

    Quote the placeholders. Sourcetrail splits the command on whitespace, so an
    unquoted path with a space in it arrives as two arguments.

  * Standalone over a whole tree, which also writes the project file:
      sourcetrail_ts_indexer.py --project-root DIR \
          --database-file-path out.srctrldb --write-project out.srctrlprj
"""

import argparse
import json
import os
import subprocess
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                                "python_indexer"))
from sourcetrail_python_indexer import (  # noqa: E402
    Db, STORAGE_VERSION, serialize_name,
    MODULE, CLASS, GLOBAL_VARIABLE, FIELD, FUNCTION, METHOD,
    MEMBER, TYPE_USAGE, CALL, INHERITANCE, IMPORT, TOKEN_LOC, SCOPE_LOC)

# src/lib/data/NodeKind.h - the kinds the Python side has no use for.
INTERFACE, ENUM, ENUM_CONSTANT, TYPEDEF = 1 << 8, 1 << 14, 1 << 15, 1 << 16

KINDS = {"module": MODULE, "class": CLASS, "interface": INTERFACE, "typedef": TYPEDEF,
         "enum": ENUM, "enumconst": ENUM_CONSTANT, "globalvar": GLOBAL_VARIABLE,
         "field": FIELD, "function": FUNCTION, "method": METHOD}
EDGES = {"call": CALL, "type": TYPE_USAGE, "use": TYPE_USAGE, "inherit": INHERITANCE}

FRONT_END = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ts_facts.mjs")
RUST_INDEXER = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                            "rust_indexer", "target", "release", "sourcetrail_rust_indexer")


def tauri_commands(crate_root, exe=RUST_INDEXER):
    """`invoke(...)` name -> serialized node name of the Rust command it reaches.

    The Rust indexer owns how a Rust symbol is named, so it is asked rather than
    imitated - an invented name would land next to the real node instead of on
    it. It rescans the crate on every call, which costs about a tenth of a
    second, so this needs no cache and no assumption about which source group
    Sourcetrail happens to index first.

    An absent crate or indexer is not an error: then there is nothing to bridge.
    """
    if not crate_root or not os.path.isdir(crate_root) or not os.path.exists(exe):
        return {}
    p = subprocess.run([exe, "--dump-commands", "--crate-root", crate_root],
                       capture_output=True, text=True)
    if p.returncode != 0:
        print(f"warning: no invoke() bridge, {os.path.basename(exe)} failed: "
              f"{p.stderr.strip()}", file=sys.stderr)
        return {}
    return json.loads(p.stdout)


def read_facts(root, targets, excludes):
    cmd = ["node", FRONT_END, "--project-root", root]
    for t in targets:
        cmd += ["--source-file-path", t]
    for e in excludes:
        cmd += ["--exclude", e]
    p = subprocess.run(cmd, capture_output=True, text=True)
    if p.returncode != 0:
        raise SystemExit(f"ts_facts.mjs failed ({p.returncode}): "
                         f"{p.stderr.strip() or p.stdout.strip()}")
    return json.loads(p.stdout)


class Writer:
    """Turns one file's facts into nodes, edges and source locations."""

    def __init__(self, db, symbols, commands=None):
        self.db, self.symbols = db, symbols
        self.commands = commands or {}

    def node_for(self, qname):
        kind, sig = self.symbols.get(qname, ("module", None))
        prefix, postfix = (sig[1], sig[0]) if sig else ("", "")
        return self.db.node(serialize_name(qname.split("."), prefix, postfix),
                            KINDS.get(kind, MODULE))

    def run(self, f):
        with open(f["path"], encoding="utf-8", errors="replace") as fh:
            src = fh.read()
        file_id = self.db.file(f["path"], src, "typescript")
        mid = self.node_for(f["mod"])
        self.db.define(mid)
        self.db.location(mid, file_id, (1, 1, f["lastLine"], f["lastCol"]), SCOPE_LOC)

        for d in f["defs"]:
            nid = self.node_for(d["q"])
            self.db.define(nid)
            self.db.edge(MEMBER, self.node_for(d["parent"]), nid)
            self.db.location(nid, file_id, d["name"], TOKEN_LOC)
            self.db.location(nid, file_id, d["scope"], SCOPE_LOC)
        for i in f["imports"]:
            tid = self.node_for(i["to"])
            self.db.edge(IMPORT, mid, tid)
            self.db.location(tid, file_id, i["at"], TOKEN_LOC)
        for r in f["refs"]:
            tid = self.node_for(r["to"])
            self.db.edge(EDGES[r["kind"]], self.node_for(r["from"]), tid)
            self.db.location(tid, file_id, r["at"], TOKEN_LOC)
        for i in f.get("invokes", []):
            name = self.commands.get(i["name"])
            if name is None:      # not a Tauri command, so not a call to anything
                continue
            tid = self.db.node(name, FUNCTION)
            self.db.edge(CALL, self.node_for(i["from"]), tid)
            self.db.location(tid, file_id, i["at"], TOKEN_LOC)


PROJECT_XML = """<?xml version="1.0" encoding="utf-8" ?>
<config>
    <source_groups>
        <source_group_5e17c093-4b28-4de6-8a55-2d9f6c3ab741>
            <custom_command>{exe} --database-file-path "%{{DATABASE_FILE_PATH}}" --database-version %{{DATABASE_VERSION}} --source-file-path "%{{SOURCE_FILE_PATH}}" --project-root "{root}"</custom_command>
            <name>TypeScript</name>
            <run_in_parallel>0</run_in_parallel>
            <source_extensions>
                <source_extension>.ts</source_extension>
                <source_extension>.tsx</source_extension>
            </source_extensions>
            <source_paths>
                <source_path>{root}</source_path>
            </source_paths>
            <status>enabled</status>
            <type>Custom Command Source Group</type>
        </source_group_5e17c093-4b28-4de6-8a55-2d9f6c3ab741>
    </source_groups>
    <version>8</version>
</config>
"""


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--database-file-path")
    p.add_argument("--source-file-path", action="append", default=[],
                   help="index only this file (repeatable); default: the whole tree")
    p.add_argument("--project-root", default=".")
    p.add_argument("--database-version", type=int)
    p.add_argument("--exclude", action="append", default=[],
                   help="skip directories containing this substring (repeatable)")
    p.add_argument("--tauri-crate", help="crate holding the #[tauri::command] functions; "
                                        "default: <project-root>/src-tauri when it exists. "
                                        "Links invoke('x') to the Rust command it calls.")
    p.add_argument("--project-file", help="store this .srctrlprj in the database, so "
                                          "Sourcetrail does not call the index outdated")
    p.add_argument("--write-project", help="also write a .srctrlprj pointing at the database")
    p.add_argument("--self-test", action="store_true")
    a = p.parse_args(argv)

    if a.self_test:
        return self_test()
    if not a.database_file_path:
        p.error("--database-file-path is required")
    if a.database_version is not None and a.database_version != STORAGE_VERSION:
        raise SystemExit(f"Sourcetrail expects storage version {a.database_version}, "
                         f"this indexer writes {STORAGE_VERSION}.")

    root = os.path.abspath(a.project_root)
    facts = read_facts(root, [os.path.abspath(s) for s in a.source_file_path], a.exclude)
    crate = a.tauri_crate or os.path.join(root, "src-tauri")
    db = Db(a.database_file_path)
    writer = Writer(db, facts["symbols"], tauri_commands(crate))
    for f in facts["files"]:
        writer.run(f)

    if a.write_project:
        xml = PROJECT_XML.format(exe=os.path.abspath(__file__), root=root)
        db.set_project_settings(xml)
        with open(a.write_project, "w", encoding="utf-8") as fh:
            fh.write(xml)
    if a.project_file:
        with open(a.project_file, encoding="utf-8") as fh:
            db.set_project_settings(fh.read())
    db.commit()
    nodes, edges = db.counts()
    print(f"indexed {len(facts['files'])} file(s) -> {nodes} nodes, {edges} edges "
          f"in {a.database_file_path}")
    return 0


def self_test():
    """One temp tree, asserting the things that break when resolution slips.

    Needs a resolvable `typescript`, so run it from the tree it indexes (or any
    directory where `node -p "require.resolve(\'typescript\')"` answers).
    """
    import tempfile
    if subprocess.run(["node", "-p", "require.resolve('typescript')"],
                      capture_output=True).returncode != 0:
        raise SystemExit("self-test needs a resolvable typescript package; "
                         "run it from the project it indexes.")
    with tempfile.TemporaryDirectory() as d:
        os.makedirs(os.path.join(d, "utils"))
        open(os.path.join(d, "utils/channels.ts"), "w").write(
            "export const TOKENS = ['a']\n"
            "export function detect(name: string): string | null {\n"
            "  return TOKENS[0] ?? name\n}\n")
        open(os.path.join(d, "store.ts"), "w").write(
            "import { detect } from './utils/channels'\n"
            "export interface Slot { id: string }\n"
            "export class Store {\n"
            "  slots: Slot[] = []\n"
            "  add(n: string) { return detect(n) }\n}\n")
        open(os.path.join(d, "Panel.tsx"), "w").write(
            "import { Store } from './store'\n"
            "import { invoke } from '@tauri-apps/api/core'\n"
            "export const Panel = () => {\n"
            "  const s = new Store()\n"
            "  invoke<string>('ping', { n: 1 })\n"
            "  invoke('not_a_command')\n"
            "  return <div>{s.add('x')}</div>\n}\n")
        os.makedirs(os.path.join(d, "src-tauri/src"))
        open(os.path.join(d, "src-tauri/Cargo.toml"), "w").write(
            '[package]\nname = "demo"\nversion = "0.1.0"\nedition = "2021"\n')
        open(os.path.join(d, "src-tauri/src/lib.rs"), "w").write(
            "#[tauri::command]\npub fn ping(n: i32) -> String { String::new() }\n"
            "pub fn helper() {}\n")
        db_path = os.path.join(d, "t.srctrldb")
        assert main(["--database-file-path", db_path, "--project-root", d]) == 0

        db = Db(db_path)
        names = {n: i for n, i in db.c.execute("SELECT serialized_name, id FROM node")}
        by_leaf = {n.split("\tn")[-1].split("\ts")[0]: i for n, i in names.items()}
        edges = {(t, s, dd) for t, s, dd in
                 db.c.execute("SELECT type, source_node_id, target_node_id FROM edge")}
        assert (CALL, by_leaf["add"], by_leaf["detect"]) in edges, "cross-file call"
        assert (CALL, by_leaf["Panel"], by_leaf["Store"]) in edges, "new Store() in .tsx"
        assert (MEMBER, by_leaf["Store"], by_leaf["add"]) in edges, "class member"
        assert (TYPE_USAGE, by_leaf["slots"], by_leaf["Slot"]) in edges, "interface type usage"
        assert (TYPE_USAGE, by_leaf["detect"], by_leaf["TOKENS"]) in edges, "same-file const"
        assert db.c.execute("SELECT COUNT(*) FROM file WHERE language='typescript'"
                            ).fetchone()[0] == 3

        # invoke('ping') reaches the Rust command; the name the edge points at
        # has to be the one the Rust indexer writes, or the graph grows a twin.
        if os.path.exists(RUST_INDEXER):
            cmds = tauri_commands(os.path.join(d, "src-tauri"))
            assert set(cmds) == {"ping"}, f"only #[tauri::command] fns: {sorted(cmds)}"
            ping = names.get(cmds["ping"])
            assert ping is not None, "invoke('ping') did not reach the Rust node name"
            assert (CALL, by_leaf["Panel"], ping) in edges, "TS -> Rust call edge"
            assert not [n for n in names if "not_a_command" in n], "unknown invoke made a node"
        else:
            print(f"skipped the invoke() bridge: {RUST_INDEXER} is not built")
        before = db.counts()

        # Sourcetrail calls the command once per file against one database.
        assert main(["--database-file-path", db_path, "--project-root", d]) == 0
        assert Db(db_path).counts() == before, "a second run duplicated symbols"
    print("self-test ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
