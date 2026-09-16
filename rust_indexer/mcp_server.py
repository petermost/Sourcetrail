#!/usr/bin/env python3
"""MCP server exposing a Sourcetrail index (.srctrldb) as code-navigation tools.

Speaks MCP over stdio: newline-delimited JSON-RPC 2.0. Stdlib only.

    mcp_server.py --db INDEX.srctrldb --crate-root DIR [--indexer PATH]
"""

import json
import os
import subprocess
import sqlite3
import sys

PROTOCOL = "2025-06-18"

# Sourcetrail packs the name hierarchy into one string; these are its delimiters.
META, NAME, PART, SIG = "\tm", "\tn", "\ts", "\tp"

NODE_KINDS = {
    1 << 0: "symbol", 1 << 1: "type", 1 << 2: "builtin", 1 << 3: "module",
    1 << 6: "struct", 1 << 8: "trait", 1 << 10: "static", 1 << 11: "field",
    1 << 12: "fn", 1 << 13: "method", 1 << 14: "enum", 1 << 15: "variant",
    1 << 16: "typedef", 1 << 17: "typeparam", 1 << 18: "file",
    1 << 19: "macro", 1 << 20: "union",
}
EDGE_KINDS = {
    1 << 0: "member", 1 << 1: "type-use", 1 << 2: "use", 1 << 3: "call",
    1 << 4: "inherits", 1 << 5: "overrides", 1 << 6: "type-arg",
    1 << 9: "import", 1 << 11: "macro-use",
}
SCOPE_LOC, TOKEN_LOC = 1, 0


def unpack(serialized):
    """Sourcetrail's serialized_name -> (readable name, signature).

    Each part is `name \\ts prefix \\tp postfix`; only the last part carries a
    signature, so a function reads back as `path::name` + `-> ret(args)`.
    """
    body = serialized.split(META, 1)[-1]
    names, prefix, postfix = [], "", ""
    for part in body.split(NAME):
        head, _, rest = part.partition(PART)
        prefix, _, postfix = rest.partition(SIG)
        names.append(head)
    if postfix:  # has a parameter list, so render it as a Rust signature
        return "::".join(names), postfix + (f" -> {prefix}" if prefix else "")
    return "::".join(names), (f": {prefix}" if prefix else "")


class Index:
    def __init__(self, db_path, crate_root, indexer):
        self.db_path, self.crate_root, self.indexer = db_path, crate_root, indexer

    def conn(self):
        if not os.path.exists(self.db_path):
            raise RuntimeError(f"no index at {self.db_path} - run the reindex tool first")
        c = sqlite3.connect(f"file:{self.db_path}?mode=ro", uri=True)
        c.row_factory = sqlite3.Row
        return c

    # -- lookups ---------------------------------------------------------
    def resolve(self, c, name):
        """Match a node by exact path, then by suffix, then by last segment."""
        rows = c.execute("SELECT id, type, serialized_name FROM node").fetchall()
        exact, suffix, leaf = [], [], []
        for r in rows:
            readable, _ = unpack(r["serialized_name"])
            if readable == name:
                exact.append((r, readable))
            elif readable.endswith("::" + name):
                suffix.append((r, readable))
            elif readable.split("::")[-1] == name:
                leaf.append((r, readable))
        return exact or suffix or leaf

    def location(self, c, element_id, kind):
        return c.execute(
            "SELECT f.path, sl.start_line, sl.end_line FROM occurrence o "
            "JOIN source_location sl ON sl.id = o.source_location_id "
            "JOIN file f ON f.id = sl.file_node_id "
            "WHERE o.element_id = ? AND sl.type = ? LIMIT 1",
            (element_id, kind),
        ).fetchone()

    def defined_at(self, c, element_id):
        # A definition is marked by its SCOPE location. Files and module
        # declarations have no scope, so fall back to the plain token.
        return self.location(c, element_id, SCOPE_LOC) or self.location(c, element_id, TOKEN_LOC)

    def rel(self, path):
        try:
            return os.path.relpath(path, self.crate_root)
        except ValueError:
            return path

    def scope(self, c, node_id):
        return c.execute(
            "SELECT sl.file_node_id, sl.start_line, sl.end_line FROM occurrence o "
            "JOIN source_location sl ON sl.id = o.source_location_id "
            "WHERE o.element_id = ? AND sl.type = ? LIMIT 1",
            (node_id, SCOPE_LOC),
        ).fetchone()

    def use_sites(self, c, holder_id, used_id):
        """Where inside `holder` does `used` appear?

        The indexer attaches locations to nodes rather than to edges, so a
        reference site is a token of the used symbol lying inside the holder's
        scope. Cheaper than re-indexing and it cannot disturb the graph view.
        """
        sc = self.scope(c, holder_id)
        if not sc:
            return []
        rows = c.execute(
            "SELECT f.path, sl.start_line FROM occurrence o "
            "JOIN source_location sl ON sl.id = o.source_location_id "
            "JOIN file f ON f.id = sl.file_node_id "
            "WHERE o.element_id = ? AND sl.type = ? AND sl.file_node_id = ? "
            "AND sl.start_line BETWEEN ? AND ? ORDER BY sl.start_line",
            (used_id, TOKEN_LOC, sc["file_node_id"], sc["start_line"], sc["end_line"]),
        ).fetchall()
        return [f"{self.rel(r['path'])}:{r['start_line']}" for r in rows]

    # -- tools -----------------------------------------------------------
    def search_symbols(self, query, kind=None, limit=30):
        with self.conn() as c:
            want = None
            if kind:
                want = [b for b, n in NODE_KINDS.items() if n == kind]
                if not want:
                    return f"unknown kind {kind!r}; known: {', '.join(sorted(set(NODE_KINDS.values())))}"
            hits = []
            for r in c.execute("SELECT id, type, serialized_name FROM node"):
                if want and r["type"] not in want:
                    continue
                readable, sig = unpack(r["serialized_name"])
                if query.lower() in readable.lower():
                    hits.append((readable, r, sig))
            hits.sort(key=lambda h: (len(h[0]), h[0]))
            if not hits:
                return f"no symbol matching {query!r}"
            out = [f"{len(hits)} match(es)" + (f", showing {limit}" if len(hits) > limit else "")]
            for readable, r, sig in hits[:limit]:
                loc = self.defined_at(c, r["id"])
                at = f"{self.rel(loc['path'])}:{loc['start_line']}" if loc else "no location"
                out.append(f"  [{NODE_KINDS.get(r['type'], r['type'])}] {readable}{sig}  -- {at}")
            return "\n".join(out)

    def symbol(self, name):
        with self.conn() as c:
            matches = self.resolve(c, name)
            if not matches:
                return f"no symbol named {name!r} - try search_symbols"
            if len(matches) > 1:
                listed = "\n".join(f"  {rd}" for _, rd in matches[:20])
                return f"{name!r} is ambiguous ({len(matches)} matches):\n{listed}"
            row, readable = matches[0]
            nid = row["id"]
            _, sig = unpack(row["serialized_name"])
            out = [f"{readable}{sig}  [{NODE_KINDS.get(row['type'], row['type'])}]"]
            loc = self.defined_at(c, nid)
            if loc:
                out.append(f"  defined: {self.rel(loc['path'])}:{loc['start_line']}-{loc['end_line']}")

            def edges(sql, other):
                groups = {}
                for e in c.execute(sql, (nid,)):
                    rd, s = unpack(e["serialized_name"])
                    holder, used = (nid, e["other_id"]) if other == "outgoing" else (e["other_id"], nid)
                    sites = self.use_sites(c, holder, used)
                    at = "  @ " + ", ".join(sites[:6]) if sites else ""
                    if len(sites) > 6:
                        at += f", +{len(sites) - 6}"
                    groups.setdefault(EDGE_KINDS.get(e["type"], e["type"]), []).append(
                        f"{rd}{s}{at}"
                    )
                for kind in sorted(groups):
                    items = sorted(set(groups[kind]))
                    out.append(f"\n  {other} -- {kind} ({len(items)}):")
                    out.extend(f"    {i}" for i in items[:40])
                    if len(items) > 40:
                        out.append(f"    ... {len(items) - 40} more")

            edges(
                "SELECT e.type, n.id AS other_id, n.serialized_name FROM edge e "
                "JOIN node n ON n.id = e.target_node_id WHERE e.source_node_id = ?",
                "outgoing",
            )
            edges(
                "SELECT e.type, n.id AS other_id, n.serialized_name FROM edge e "
                "JOIN node n ON n.id = e.source_node_id WHERE e.target_node_id = ?",
                "incoming",
            )
            return "\n".join(out)

    def file_symbols(self, path):
        with self.conn() as c:
            rows = c.execute(
                "SELECT n.serialized_name, n.type, sl.start_line, sl.end_line, f.path "
                "FROM source_location sl "
                "JOIN file f ON f.id = sl.file_node_id "
                "JOIN occurrence o ON o.source_location_id = sl.id "
                "JOIN node n ON n.id = o.element_id "
                "WHERE sl.type = ? AND f.path LIKE ? ORDER BY f.path, sl.start_line",
                (SCOPE_LOC, f"%{path}%"),
            ).fetchall()
            if not rows:
                return f"no indexed definitions in a file matching {path!r}"
            out, current = [], None
            for r in rows:
                if r["path"] != current:
                    current = r["path"]
                    out.append(self.rel(current))
                readable, sig = unpack(r["serialized_name"])
                out.append(
                    f"  {r['start_line']:>5}-{r['end_line']:<5} "
                    f"[{NODE_KINDS.get(r['type'], r['type'])}] {readable}{sig}"
                )
            return "\n".join(out)

    def reindex(self):
        # Rebuild from scratch: resuming would keep locations for code that is gone.
        if os.path.exists(self.db_path):
            os.remove(self.db_path)
        for extra in (".srctrldb-wal", ".srctrldb-shm"):
            stale = self.db_path.replace(".srctrldb", extra)
            if os.path.exists(stale):
                os.remove(stale)
        p = subprocess.run(
            [self.indexer, "--database-file-path", self.db_path, "--crate-root", self.crate_root],
            capture_output=True, text=True, timeout=600,
        )
        if p.returncode != 0:
            return f"indexer failed ({p.returncode}):\n{p.stderr.strip() or p.stdout.strip()}"
        with self.conn() as c:
            n = c.execute("SELECT COUNT(*) FROM node").fetchone()[0]
            e = c.execute("SELECT COUNT(*) FROM edge").fetchone()[0]
            f = c.execute("SELECT COUNT(*) FROM file").fetchone()[0]
            err = c.execute("SELECT COUNT(*) FROM error").fetchone()[0]
        return (f"reindexed {self.crate_root}\n  {f} files, {n} symbols, {e} references, "
                f"{err} errors\nReopen or refresh the project in Sourcetrail to see it.")


TOOLS = [
    {
        "name": "search_symbols",
        "description": "Find indexed Rust symbols whose full path contains a substring. "
                       "Use this first when you only know part of a name.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "query": {"type": "string", "description": "substring of the symbol path"},
                "kind": {"type": "string", "description": "optional filter: "
                         + ", ".join(sorted(set(NODE_KINDS.values())))},
                "limit": {"type": "integer", "description": "max results (default 30)"},
            },
            "required": ["query"],
        },
    },
    {
        "name": "symbol",
        "description": "Everything the index knows about one symbol: kind, signature, where it is "
                       "defined, what it calls or uses, and what calls or uses it. Accepts a full "
                       "path or a unique short name.",
        "inputSchema": {
            "type": "object",
            "properties": {"name": {"type": "string"}},
            "required": ["name"],
        },
    },
    {
        "name": "file_symbols",
        "description": "List every symbol defined in a source file, with line ranges. "
                       "Path may be a fragment such as 'commands/asset.rs'.",
        "inputSchema": {
            "type": "object",
            "properties": {"path": {"type": "string"}},
            "required": ["path"],
        },
    },
    {
        "name": "reindex",
        "description": "Re-run the Rust indexer over the crate and rebuild the index from scratch. "
                       "Call after source changes so lookups reflect the current code.",
        "inputSchema": {"type": "object", "properties": {}},
    },
]


def handle(req, index):
    method, params = req.get("method"), req.get("params") or {}
    if method == "initialize":
        asked = params.get("protocolVersion")
        return {
            "protocolVersion": asked if isinstance(asked, str) else PROTOCOL,
            "capabilities": {"tools": {}},
            "serverInfo": {"name": "sourcetrail-rust", "version": "0.1.0"},
        }
    if method == "ping":
        return {}
    if method == "tools/list":
        return {"tools": TOOLS}
    if method == "tools/call":
        name = params.get("name")
        args = params.get("arguments") or {}
        fn = getattr(index, name, None) if any(t["name"] == name for t in TOOLS) else None
        if fn is None:
            raise LookupError(f"unknown tool {name!r}")
        try:
            text = fn(**args)
            return {"content": [{"type": "text", "text": text}]}
        except Exception as exc:  # surfaced to the model, not a transport failure
            return {"content": [{"type": "text", "text": f"{type(exc).__name__}: {exc}"}],
                    "isError": True}
    raise LookupError(f"unknown method {method!r}")


def serve(index, stdin=sys.stdin, stdout=sys.stdout):
    for line in stdin:
        line = line.strip()
        if not line:
            continue
        req = json.loads(line)
        if "id" not in req:  # notification
            continue
        try:
            resp = {"jsonrpc": "2.0", "id": req["id"], "result": handle(req, index)}
        except LookupError as exc:
            resp = {"jsonrpc": "2.0", "id": req["id"],
                    "error": {"code": -32601, "message": str(exc)}}
        except Exception as exc:
            resp = {"jsonrpc": "2.0", "id": req["id"],
                    "error": {"code": -32603, "message": f"{type(exc).__name__}: {exc}"}}
        stdout.write(json.dumps(resp) + "\n")
        stdout.flush()


def self_test(index):
    packed = "::" + META + "asset_bridge" + NAME + "run" + PART + "Result<()>" + SIG + "(argc: i32)"
    assert unpack(packed) == ("asset_bridge::run", "(argc: i32) -> Result<()>"), unpack(packed)
    field = "::" + META + "Cfg" + NAME + "name" + PART + "String" + SIG
    assert unpack(field) == ("Cfg::name", ": String"), unpack(field)
    assert unpack("/" + META + "/src/lib.rs" + PART + SIG) == ("/src/lib.rs", "")

    import io
    session = [
        {"jsonrpc": "2.0", "id": 1, "method": "initialize",
         "params": {"protocolVersion": PROTOCOL}},
        {"jsonrpc": "2.0", "method": "notifications/initialized"},
        {"jsonrpc": "2.0", "id": 2, "method": "tools/list"},
        {"jsonrpc": "2.0", "id": 3, "method": "tools/call",
         "params": {"name": "search_symbols", "arguments": {"query": "zzz_no_such_symbol"}}},
        {"jsonrpc": "2.0", "id": 4, "method": "tools/call", "params": {"name": "nope"}},
    ]
    out = io.StringIO()
    serve(index, io.StringIO("\n".join(json.dumps(m) for m in session)), out)
    got = [json.loads(l) for l in out.getvalue().splitlines()]
    assert [r["id"] for r in got] == [1, 2, 3, 4], "notifications must not get a response"
    assert got[0]["result"]["protocolVersion"] == PROTOCOL
    assert {t["name"] for t in got[1]["result"]["tools"]} == {
        "search_symbols", "symbol", "file_symbols", "reindex"}
    assert "no symbol" in got[2]["result"]["content"][0]["text"]
    assert got[3]["error"]["code"] == -32601
    print("self-test ok")


def main(argv):
    args = dict(zip(argv[::2], argv[1::2]))
    db = args.get("--db")
    root = args.get("--crate-root")
    if not db or not root:
        sys.exit(__doc__)
    here = os.path.dirname(os.path.abspath(__file__))
    indexer = args.get("--indexer", os.path.join(here, "target/release/sourcetrail_rust_indexer"))
    index = Index(os.path.abspath(db), os.path.abspath(root), indexer)
    (self_test if "--self-test" in argv else serve)(index)


if __name__ == "__main__":
    main(sys.argv[1:])
