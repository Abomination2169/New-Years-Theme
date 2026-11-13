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
main_source = (root / "src/main.cpp").read_text()
audio_source = (root / "src/Audio/ambience.cpp").read_text()
assert "NewYearsSongPreviewOnEnable" in main_source
assert "NewYearsSongPreviewCrossfade" in main_source
assert "PlayNewYearsMenuSong" in audio_source
assert "_defaultAudioClip = clip" in audio_source
assert "NewYearsDefaultMenuMusic" not in main_source
assert "DontDestroyOnLoad" not in audio_source
for e in manifest["fileCopies"]:
    assert (root / "FileCopies" / e["name"]).is_file(), e
print("PASS: New Year's 1.40.8 metadata and assets; Halloween-style native menu music replacement")
