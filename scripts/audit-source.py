#!/usr/bin/env python3
import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
qpm = json.loads((root / "qpm.json").read_text())
shared = json.loads((root / "qpm.shared.json").read_text())
manifest = json.loads((root / "mod.template.json").read_text())
assert qpm["info"]["id"] == shared["config"]["info"]["id"] == manifest["id"] == "newyears"
assert manifest["packageVersion"] == "1.40.8_7379"
assert manifest["version"] == qpm["info"]["version"] == "1.0.0"
assert qpm["info"]["additionalData"]["overrideSoName"] == "libnewyears.so"
assert (root / manifest["coverImage"]).is_file()
assert (root / "assets/confetti_streamers.png").is_file()
assert (root / "src/Confetti/confetti.cpp").is_file()
assert "SongPreviewPlayer::" not in (root / "src/main.cpp").read_text()
for e in manifest["fileCopies"]:
    assert (root / "FileCopies" / e["name"]).is_file(), e
print("PASS: New Year's 1.40.8 metadata and assets; stock menu music unchanged")
