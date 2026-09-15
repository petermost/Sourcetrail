//! Writer for Sourcetrail's SQLite index format (storage version 25).
//!
//! Mirrors `src/lib/data/storage/sqlite/SqliteIndexStorage.cpp` of this repo.
//! Either appends to the per-thread database Sourcetrail hands to a
//! "Custom Command" source group, or creates a standalone `.srctrldb`.

use anyhow::Result;
use rusqlite::{params, Connection, OptionalExtension};
use std::collections::HashMap;
use std::path::Path;

pub const STORAGE_VERSION: i64 = 25;

/// `NodeKind` in src/lib/data/NodeKind.h
pub mod node {
    pub const SYMBOL: i32 = 1 << 0;
    pub const TYPE: i32 = 1 << 1;
    pub const BUILTIN_TYPE: i32 = 1 << 2;
    pub const MODULE: i32 = 1 << 3;
    pub const STRUCT: i32 = 1 << 6;
    pub const INTERFACE: i32 = 1 << 8;
    pub const GLOBAL_VARIABLE: i32 = 1 << 10;
    pub const FIELD: i32 = 1 << 11;
    pub const FUNCTION: i32 = 1 << 12;
    pub const METHOD: i32 = 1 << 13;
    pub const ENUM: i32 = 1 << 14;
    pub const ENUM_CONSTANT: i32 = 1 << 15;
    pub const TYPEDEF: i32 = 1 << 16;
    pub const TYPE_PARAMETER: i32 = 1 << 17;
    pub const FILE: i32 = 1 << 18;
    pub const MACRO: i32 = 1 << 19;
    pub const UNION: i32 = 1 << 20;
}

/// `Edge::EdgeType` in src/lib/data/graph/Edge.h
pub mod edge {
    pub const MEMBER: i32 = 1 << 0;
    pub const TYPE_USAGE: i32 = 1 << 1;
    pub const USAGE: i32 = 1 << 2;
    pub const CALL: i32 = 1 << 3;
    pub const INHERITANCE: i32 = 1 << 4;
    pub const OVERRIDE: i32 = 1 << 5;
    pub const TYPE_ARGUMENT: i32 = 1 << 6;
    pub const IMPORT: i32 = 1 << 9;
    pub const MACRO_USAGE: i32 = 1 << 11;
}

/// `LocationType` in src/lib/data/location/LocationType.h
pub mod loc {
    pub const TOKEN: i32 = 0;
    pub const SCOPE: i32 = 1;
    pub const QUALIFIER: i32 = 2;
    pub const LOCAL_SYMBOL: i32 = 3;
    pub const SIGNATURE: i32 = 4;
    pub const ERROR: i32 = 6;
}

/// `DefinitionKind` in src/lib/data/DefinitionKind.h
pub const DEF_EXPLICIT: i32 = 2;

/// 1-based, end-inclusive — same convention Sourcetrail uses.
#[derive(Clone, Copy, Debug)]
pub struct Range {
    pub start_line: usize,
    pub start_col: usize,
    pub end_line: usize,
    pub end_col: usize,
}

pub struct Db {
    conn: Connection,
    next_element: i64,
    next_location: i64,
    nodes: HashMap<String, i64>,
    edges: HashMap<(i32, i64, i64), i64>,
    locals: HashMap<String, i64>,
}

impl Db {
    pub fn open(path: &Path) -> Result<Self> {
        let conn = Connection::open(path)?;
        conn.busy_timeout(std::time::Duration::from_secs(60))?;
        conn.pragma_update(None, "journal_mode", "WAL")?;
        conn.pragma_update(None, "synchronous", "OFF")?;
        conn.pragma_update(None, "foreign_keys", "ON")?;

        let mut db = Db {
            conn,
            next_element: 1,
            next_location: 1,
            nodes: HashMap::new(),
            edges: HashMap::new(),
            locals: HashMap::new(),
        };
        db.setup()?;
        db.resume_ids()?;
        Ok(db)
    }

    /// `CREATE TABLE IF NOT EXISTS` — a no-op when Sourcetrail created the file.
    fn setup(&mut self) -> Result<()> {
        self.conn.execute_batch(
            r#"
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
            "#,
        )?;
        let has: Option<String> = self
            .conn
            .query_row("SELECT value FROM meta WHERE key = 'storage_version'", [], |r| r.get(0))
            .optional()?;
        if has.is_none() {
            self.conn.execute(
                "INSERT INTO meta(id, key, value) VALUES(NULL, 'storage_version', ?1)",
                params![STORAGE_VERSION.to_string()],
            )?;
        }
        Ok(())
    }

    /// Continue id allocation after whatever Sourcetrail (or an earlier file) wrote.
    fn resume_ids(&mut self) -> Result<()> {
        self.next_element = self
            .conn
            .query_row("SELECT IFNULL(MAX(id), 0) + 1 FROM element", [], |r| r.get(0))?;
        self.next_location = self
            .conn
            .query_row("SELECT IFNULL(MAX(id), 0) + 1 FROM source_location", [], |r| r.get(0))?;
        let mut stmt = self.conn.prepare("SELECT serialized_name, id FROM node")?;
        for row in stmt.query_map([], |r| Ok((r.get::<_, String>(0)?, r.get::<_, i64>(1)?)))? {
            let (name, id) = row?;
            self.nodes.insert(name, id);
        }
        Ok(())
    }

    pub fn begin(&self) -> Result<()> {
        self.conn.execute_batch("BEGIN")?;
        Ok(())
    }

    pub fn commit(&self) -> Result<()> {
        self.conn.execute_batch("COMMIT")?;
        Ok(())
    }

    fn new_element(&mut self) -> Result<i64> {
        let id = self.next_element;
        self.next_element += 1;
        self.conn.execute("INSERT INTO element(id) VALUES(?1)", params![id])?;
        Ok(id)
    }

    /// Insert a node, or return the existing one with the same serialized name.
    /// A later, more specific kind wins — a symbol first seen as a bare reference
    /// gets upgraded once its definition is indexed.
    pub fn node(&mut self, serialized_name: &str, kind: i32) -> Result<i64> {
        if let Some(&id) = self.nodes.get(serialized_name) {
            if kind != node::SYMBOL && kind != node::TYPE {
                let current: i32 =
                    self.conn.query_row("SELECT type FROM node WHERE id = ?1", params![id], |r| r.get(0))?;
                if current == node::SYMBOL || current == node::TYPE {
                    self.conn
                        .execute("UPDATE node SET type = ?1 WHERE id = ?2", params![kind, id])?;
                }
            }
            return Ok(id);
        }
        let id = self.new_element()?;
        self.conn.execute(
            "INSERT INTO node(id, type, serialized_name) VALUES(?1, ?2, ?3)",
            params![id, kind, serialized_name],
        )?;
        self.nodes.insert(serialized_name.to_string(), id);
        Ok(id)
    }

    /// Mark a node as actually defined here (not just referenced).
    pub fn define(&mut self, node_id: i64) -> Result<()> {
        self.conn.execute(
            "INSERT OR REPLACE INTO symbol(id, definition_kind) VALUES(?1, ?2)",
            params![node_id, DEF_EXPLICIT],
        )?;
        Ok(())
    }

    pub fn file(&mut self, path: &str, line_count: usize, content: &str, mtime: &str) -> Result<i64> {
        let id = self.node(&serialize_file_name(path), node::FILE)?;
        let exists: Option<i64> = self
            .conn
            .query_row("SELECT id FROM file WHERE id = ?1", params![id], |r| r.get(0))
            .optional()?;
        if exists.is_none() {
            self.conn.execute(
                "INSERT INTO file(id, path, language, modification_time, indexed, complete, line_count)
                 VALUES(?1, ?2, 'rust', ?3, 1, 1, ?4)",
                params![id, path, mtime, line_count as i64],
            )?;
            self.conn.execute(
                "INSERT INTO filecontent(id, content) VALUES(?1, ?2)",
                params![id, content],
            )?;
        }
        Ok(id)
    }

    pub fn edge(&mut self, kind: i32, source: i64, target: i64) -> Result<i64> {
        if source == target {
            return Ok(0);
        }
        if let Some(&id) = self.edges.get(&(kind, source, target)) {
            return Ok(id);
        }
        let id = self.new_element()?;
        self.conn.execute(
            "INSERT INTO edge(id, type, source_node_id, target_node_id) VALUES(?1, ?2, ?3, ?4)",
            params![id, kind, source, target],
        )?;
        self.edges.insert((kind, source, target), id);
        Ok(id)
    }

    pub fn local(&mut self, name: &str) -> Result<i64> {
        if let Some(&id) = self.locals.get(name) {
            return Ok(id);
        }
        let id = self.new_element()?;
        self.conn
            .execute("INSERT INTO local_symbol(id, name) VALUES(?1, ?2)", params![id, name])?;
        self.locals.insert(name.to_string(), id);
        Ok(id)
    }

    pub fn location(&mut self, element_id: i64, file_id: i64, r: Range, kind: i32) -> Result<()> {
        if element_id == 0 {
            return Ok(());
        }
        let id = self.next_location;
        self.next_location += 1;
        self.conn.execute(
            "INSERT INTO source_location(id, file_node_id, start_line, start_column, end_line, end_column, type)
             VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7)",
            params![id, file_id, r.start_line as i64, r.start_col as i64, r.end_line as i64, r.end_col as i64, kind],
        )?;
        self.conn.execute(
            "INSERT OR IGNORE INTO occurrence(element_id, source_location_id) VALUES(?1, ?2)",
            params![element_id, id],
        )?;
        Ok(())
    }

    pub fn error(&mut self, message: &str, file: &str, r: Range) -> Result<()> {
        let id = self.new_element()?;
        self.conn.execute(
            "INSERT INTO error(id, message, fatal, indexed, translation_unit) VALUES(?1, ?2, 0, 1, ?3)",
            params![id, message, file],
        )?;
        let file_id = self.nodes.get(&serialize_file_name(file)).copied().unwrap_or(0);
        if file_id != 0 {
            self.location(id, file_id, r, loc::ERROR)?;
        }
        Ok(())
    }

    pub fn set_project_settings(&mut self, xml: &str) -> Result<()> {
        self.conn.execute(
            "INSERT OR REPLACE INTO meta(id, key, value)
             VALUES((SELECT id FROM meta WHERE key = 'project_settings'), 'project_settings', ?1)",
            params![xml],
        )?;
        Ok(())
    }

    pub fn counts(&self) -> Result<(i64, i64)> {
        Ok((
            self.conn.query_row("SELECT COUNT(*) FROM node", [], |r| r.get(0))?,
            self.conn.query_row("SELECT COUNT(*) FROM edge", [], |r| r.get(0))?,
        ))
    }
}

/// `NameHierarchy::serialize` — see src/lib/data/name/NameHierarchy.cpp.
/// Layout: `<delimiter>\tm<name>\ts<prefix>\tp<postfix>` joined by `\tn`.
pub fn serialize_name(parts: &[&str], prefix: &str, postfix: &str) -> String {
    let mut s = String::from("::\tm");
    for (i, part) in parts.iter().enumerate() {
        if i > 0 {
            s.push_str("\tn");
        }
        let last = i + 1 == parts.len();
        s.push_str(part);
        s.push_str("\ts");
        if last {
            s.push_str(prefix);
        }
        s.push_str("\tp");
        if last {
            s.push_str(postfix);
        }
    }
    s
}

/// Files use the `/` delimiter and a single name part holding the whole path.
pub fn serialize_file_name(path: &str) -> String {
    format!("/\tm{path}\ts\tp")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn name_serialization_matches_sourcetrail() {
        assert_eq!(
            serialize_name(&["asset_bridge", "commands", "load"], "Result<()>", "(p: String)"),
            "::\tmasset_bridge\ts\tp\tncommands\ts\tp\tnload\tsResult<()>\tp(p: String)"
        );
        assert_eq!(serialize_name(&["Foo"], "", ""), "::\tmFoo\ts\tp");
        assert_eq!(serialize_file_name("/a/b.rs"), "/\tm/a/b.rs\ts\tp");
    }
}
