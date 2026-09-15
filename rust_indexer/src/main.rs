//! Rust indexer for Sourcetrail.
//!
//! Two ways to run it:
//!
//!   * As a Sourcetrail "Custom Command" source group, once per source file:
//!       sourcetrail_rust_indexer --database-file-path %{DATABASE_FILE_PATH} \
//!           --database-version %{DATABASE_VERSION} --source-file-path %{SOURCE_FILE_PATH}
//!
//!   * Standalone over a whole crate, which also writes the project file:
//!       sourcetrail_rust_indexer --crate-root path/to/crate \
//!           --database-file-path out.srctrldb --write-project out.srctrlprj

mod db;
mod index;

use anyhow::{bail, Context, Result};
use std::path::{Path, PathBuf};

struct Args {
    database: PathBuf,
    sources: Vec<PathBuf>,
    crate_root: Option<PathBuf>,
    database_version: Option<i64>,
    write_project: Option<PathBuf>,
}

fn parse_args() -> Result<Args> {
    let mut a = Args {
        database: PathBuf::new(),
        sources: Vec::new(),
        crate_root: None,
        database_version: None,
        write_project: None,
    };
    let mut it = std::env::args().skip(1);
    while let Some(flag) = it.next() {
        let mut value = || it.next().with_context(|| format!("{flag} needs a value"));
        match flag.as_str() {
            "--database-file-path" => a.database = PathBuf::from(value()?),
            "--source-file-path" => a.sources.push(PathBuf::from(value()?)),
            "--crate-root" | "--project-root" => a.crate_root = Some(PathBuf::from(value()?)),
            "--database-version" => a.database_version = value()?.parse().ok(),
            "--write-project" => a.write_project = Some(PathBuf::from(value()?)),
            "-h" | "--help" => {
                println!("{}", HELP);
                std::process::exit(0);
            }
            other => bail!("unknown argument: {other}"),
        }
    }
    if a.database.as_os_str().is_empty() {
        bail!("--database-file-path is required\n\n{HELP}");
    }
    Ok(a)
}

const HELP: &str = "\
sourcetrail_rust_indexer --database-file-path <db> [options]

  --source-file-path <file>   index only this file (repeatable); default: whole crate
  --crate-root <dir>          crate directory holding Cargo.toml
  --database-version <n>      Sourcetrail's storage version, checked against this build
  --write-project <file>      also write a .srctrlprj pointing at the database";

fn main() -> Result<()> {
    let args = parse_args()?;

    if let Some(v) = args.database_version {
        if v != db::STORAGE_VERSION {
            bail!(
                "Sourcetrail expects storage version {v}, this indexer writes {}. \
                 Rebuild the indexer against the matching Sourcetrail sources.",
                db::STORAGE_VERSION
            );
        }
    }

    let crate_root = match &args.crate_root {
        Some(p) => p.clone(),
        None => {
            let start = args.sources.first().cloned().unwrap_or_else(|| PathBuf::from("."));
            find_crate_root(&start)
                .context("no Cargo.toml found; pass --crate-root explicitly")?
        }
    };
    let src_root = crate_root.join("src");
    if !src_root.is_dir() {
        bail!("{} has no src/ directory", crate_root.display());
    }
    let name = crate_name(&crate_root)?;

    let index = index::CrateIndex::scan(&src_root, &name)
        .with_context(|| format!("scanning {}", src_root.display()))?;

    let targets: Vec<PathBuf> = if args.sources.is_empty() {
        index.files.clone()
    } else {
        args.sources
            .iter()
            .map(|p| p.canonicalize().unwrap_or_else(|_| p.clone()))
            .collect()
    };

    let mut database = db::Db::open(&args.database)
        .with_context(|| format!("opening {}", args.database.display()))?;
    database.begin()?;
    for file in &targets {
        index::emit_file(&mut database, &index, file)
            .with_context(|| format!("indexing {}", file.display()))?;
    }

    if let Some(project) = &args.write_project {
        let xml = project_xml(&src_root);
        database.set_project_settings(&xml)?;
        std::fs::write(project, &xml)?;
    }
    database.commit()?;

    let (nodes, edges) = database.counts()?;
    println!(
        "indexed {} file(s) of crate '{name}' -> {nodes} nodes, {edges} edges in {}",
        targets.len(),
        args.database.display()
    );
    Ok(())
}

fn find_crate_root(start: &Path) -> Option<PathBuf> {
    let mut dir = start.canonicalize().ok()?;
    if dir.is_file() {
        dir.pop();
    }
    loop {
        if dir.join("Cargo.toml").is_file() {
            return Some(dir);
        }
        if !dir.pop() {
            return None;
        }
    }
}

/// Reads `name` from `[package]`. A full TOML parser would be a dependency for
/// one line of text.
fn crate_name(crate_root: &Path) -> Result<String> {
    let manifest = std::fs::read_to_string(crate_root.join("Cargo.toml"))
        .with_context(|| format!("reading {}/Cargo.toml", crate_root.display()))?;
    let mut in_package = false;
    for line in manifest.lines() {
        let line = line.trim();
        if line.starts_with('[') {
            in_package = line == "[package]";
            continue;
        }
        if in_package {
            if let Some(rest) = line.strip_prefix("name") {
                if let Some(v) = rest.split('=').nth(1) {
                    return Ok(v.trim().trim_matches('"').replace('-', "_"));
                }
            }
        }
    }
    bail!("no [package] name in {}/Cargo.toml", crate_root.display())
}

/// A Custom Command source group wired to this binary, so Sourcetrail's
/// refresh button re-runs the indexer.
fn project_xml(src_root: &Path) -> String {
    let exe = std::env::current_exe()
        .map(|p| p.display().to_string())
        .unwrap_or_else(|_| "sourcetrail_rust_indexer".to_string());
    format!(
        r#"<?xml version="1.0" encoding="utf-8" ?>
<config>
    <source_groups>
        <source_group_9d2f7b1e-0c44-4a6b-9f31-6b1c2a7e51d0>
            <custom_command>{exe} --database-file-path %{{DATABASE_FILE_PATH}} --database-version %{{DATABASE_VERSION}} --source-file-path %{{SOURCE_FILE_PATH}}</custom_command>
            <name>Rust</name>
            <run_in_parallel>0</run_in_parallel>
            <source_extensions>
                <source_extension>.rs</source_extension>
            </source_extensions>
            <source_paths>
                <source_path>{src}</source_path>
            </source_paths>
            <status>enabled</status>
            <type>Custom Command Source Group</type>
        </source_group_9d2f7b1e-0c44-4a6b-9f31-6b1c2a7e51d0>
    </source_groups>
    <version>8</version>
</config>
"#,
        src = src_root.display()
    )
}
