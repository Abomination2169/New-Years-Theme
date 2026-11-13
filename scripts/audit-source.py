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
assert (root / "src/Fireworks/fireworks.cpp").is_file()
assert (root / "assets/new_year_crystal_ball.png").is_file()
assert not (root / "FileCopies/Midnight_Synthwave_Tokyo.ogg").exists()
main_source = (root / "src/main.cpp").read_text()
assert "INSTALL_HOOK(PaperLogger, NewYearsMenuMusicOnEnable)" in main_source
assert "INSTALL_HOOK(PaperLogger, NewYearsMenuMusicCrossfade)" in main_source
assert "NewYearsRememberMenuPlayer(self)" in main_source
assert "NewYearsMainMenuActivated()" in main_source
assert "NewYears::fireworks::Create()" in main_source
assert "PlayNewYearsMenuSong" not in main_source
assert "NewYearsDefaultMenuMusic" not in main_source
assert manifest["fileCopies"] == []
for e in manifest["fileCopies"]:
    assert (root / "FileCopies" / e["name"]).is_file(), e
print("PASS: New Year's 1.40.8 metadata and assets; guarded menu audio with native song-preview transitions")
