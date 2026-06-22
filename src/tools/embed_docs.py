# -*- coding: utf-8 -*-
"""
embed_docs.py — Offline RAG embedding script (Step 3)

Chunks project PDFs and Markdown files, generates embeddings via
nomic-embed-text through Ollama, and saves them to a SQLite database
that the Qt app reads at runtime for context retrieval.

Requirements:
    pip install pymupdf requests

Usage:
    # Make sure Ollama is running with nomic-embed-text:
    #   ollama pull nomic-embed-text
    #   ollama serve
    python embed_docs.py

Output:
    src/rag/vector.db   (~10-30 MB)
"""

import sqlite3
import struct
import os
import re
import json
import requests
import argparse

# ── Config ────────────────────────────────────────────────────────────────────

OLLAMA_URL    = "http://127.0.0.1:11434"
EMBED_MODEL   = "nomic-embed-text"
CHUNK_SIZE    = 400   # tokens (approx chars / 4)
CHUNK_OVERLAP = 80
DB_PATH       = os.path.join(os.path.dirname(__file__), "..", "rag", "vector.db")

SCRIPT_DIR    = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT  = os.path.normpath(os.path.join(SCRIPT_DIR, "..", ".."))
SKILL_ASSETS  = os.path.join(PROJECT_ROOT, ".agents", "skills", "time-grapher", "assets")
DOCS_ROOT     = os.path.join(PROJECT_ROOT, "docs")

SOURCES = [
    # (path, label)
    (os.path.join(SKILL_ASSETS, "Witschi-Training-Course.pdf"), "witschi-training"),
    (os.path.join(SKILL_ASSETS, "Witschi Chronoscope X1 G3 Instruction Manual.pdf"), "witschi-manual"),
    (os.path.join(SKILL_ASSETS, "TimeGrapher Equations_v0.docx.pdf"), "tg-equations"),
    (os.path.join(DOCS_ROOT, "ai-features.md"), "project-ai-features"),
    (os.path.join(DOCS_ROOT, "metrics-explained.md"), "project-metrics"),
    (os.path.join(DOCS_ROOT, "week1", "kickoff-workshop", "domain-knowledge.md"), "project-domain"),
]

# ── Helpers ───────────────────────────────────────────────────────────────────

def read_pdf(path: str) -> str:
    import fitz  # PyMuPDF
    doc = fitz.open(path)
    return "\n".join(page.get_text() for page in doc)


def read_md(path: str) -> str:
    with open(path, encoding="utf-8") as f:
        return f.read()


def read_source(path: str) -> str:
    ext = os.path.splitext(path)[1].lower()
    if ext == ".pdf":
        return read_pdf(path)
    return read_md(path)


def chunk_text(text: str, chunk_chars: int, overlap_chars: int):
    """Split text into overlapping chunks by character count."""
    text = re.sub(r"\n{3,}", "\n\n", text).strip()
    chunks = []
    start = 0
    while start < len(text):
        end = min(start + chunk_chars, len(text))
        # try to break at paragraph boundary
        boundary = text.rfind("\n\n", start, end)
        if boundary > start + chunk_chars // 2:
            end = boundary
        chunk = text[start:end].strip()
        if len(chunk) > 80:   # skip tiny fragments
            chunks.append(chunk)
        next_start = end - overlap_chars
        if next_start <= start:   # always advance to avoid infinite loop
            next_start = end
        start = next_start
    return chunks


def sanitize(text: str) -> str:
    # remove null bytes and non-printable control chars that trip up Ollama
    text = text.replace("\x00", " ")
    text = re.sub(r"[\x01-\x08\x0b\x0c\x0e-\x1f\x7f]", " ", text)
    return text.strip()


def embed(text: str) -> list[float] | None:
    text = sanitize(text)
    if not text:
        return None
    try:
        resp = requests.post(
            f"{OLLAMA_URL}/api/embeddings",
            json={"model": EMBED_MODEL, "prompt": text[:2000]},  # hard cap
            timeout=60,
        )
        resp.raise_for_status()
        return resp.json()["embedding"]
    except Exception as e:
        print(f"  [warn] embed failed: {e} — skipping chunk")
        return None


def pack_embedding(vec: list[float]) -> bytes:
    return struct.pack(f"{len(vec)}f", *vec)


def unpack_embedding(data: bytes) -> list[float]:
    n = len(data) // 4
    return list(struct.unpack(f"{n}f", data))


# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--db", default=DB_PATH, help="Output SQLite path")
    args = parser.parse_args()

    os.makedirs(os.path.dirname(os.path.abspath(args.db)), exist_ok=True)

    # verify Ollama + model
    try:
        tags = requests.get(f"{OLLAMA_URL}/api/tags", timeout=5).json()
        names = [m["name"] for m in tags.get("models", [])]
        if not any(EMBED_MODEL in n for n in names):
            print(f"[!] Model '{EMBED_MODEL}' not found. Run: ollama pull {EMBED_MODEL}")
            return
    except Exception as e:
        print(f"[!] Ollama not reachable: {e}\n    Run: ollama serve")
        return

    conn = sqlite3.connect(args.db)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS chunks (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            source    TEXT    NOT NULL,
            text      TEXT    NOT NULL,
            embedding BLOB    NOT NULL
        )
    """)
    conn.execute("CREATE INDEX IF NOT EXISTS idx_source ON chunks(source)")
    conn.commit()

    chunk_chars   = CHUNK_SIZE * 4
    overlap_chars = CHUNK_OVERLAP * 4

    for path, label in SOURCES:
        path = os.path.normpath(path)
        if not os.path.exists(path):
            print(f"[skip] not found: {path}")
            continue

        # clear existing chunks for this source so re-runs are idempotent
        conn.execute("DELETE FROM chunks WHERE source = ?", (label,))
        conn.commit()

        print(f"[{label}] reading {os.path.basename(path)} ...")
        text   = read_source(path)
        chunks = chunk_text(text, chunk_chars, overlap_chars)
        print(f"[{label}] {len(chunks)} chunks → embedding ...")

        for i, chunk in enumerate(chunks):
            vec = embed(chunk)
            if vec is None:
                continue
            blob = pack_embedding(vec)
            conn.execute(
                "INSERT INTO chunks (source, text, embedding) VALUES (?, ?, ?)",
                (label, chunk, blob),
            )
            if (i + 1) % 10 == 0:
                conn.commit()
                print(f"  {i+1}/{len(chunks)}")

        conn.commit()
        print(f"[{label}] done.")

    total = conn.execute("SELECT COUNT(*) FROM chunks").fetchone()[0]
    size  = os.path.getsize(args.db) // 1024
    print(f"\nDone. {total} chunks → {args.db} ({size} KB)")
    conn.close()


if __name__ == "__main__":
    main()
