# 🎆 New Year's Theme — Beat Saber Quest

A seasonal menu theme for **Meta Quest 3 · Beat Saber 1.40.8_7379**.

## Features
- Detailed illuminated New Year's ball with a **Happy New Year!** banner
- Midnight-blue menu lighting and falling confetti
- Native Beat Saber fireworks controller integration (uses existing game effects where available)
- Menu music selected from your **installed SongCore custom songs** (`.egg` or `.ogg`), without bundling copyrighted tracks
- Song previews remain audible; custom menu music resumes after previews
- Menu music starts when the main menu activates, rather than during early audio initialization
- Independent config switches for confetti, fireworks, lighting and menu music

## Install
Install the release QMOD with a compatible Quest mod manager on **Beat Saber 1.40.8_7379**. SongCore and installed custom songs are needed for custom menu music; otherwise Beat Saber's normal menu audio remains available. Do not install simultaneously with another theme that modifies the same menu environment unless coordinated through Holiday Theme Switcher.

## Build from source
Use the included GitHub Actions workflow, or install QPM CLI and the Quest Android NDK, then run `qpm restore`, `qpm qmod manifest`, configure CMake and build, and run `qpm qmod zip --import mod.json --skip_build`. The workflow validates the packaged QMOD and publishes both the QMOD and a source archive for version tags.

## Compatibility and limitations
The mod is tested on Quest 3 with Beat Saber 1.40.8. The fireworks integration enables an existing native controller when available; it does not synthesize new fireworks or guarantee audio in every environment. Automatic advancement between menu tracks is not yet implemented. Test before using with other environment-changing mods.

## Credits and licensing
Created by **Abomination2169** with AI assistance. Derived from [Halloween Theme](https://github.com/Abomination2169/Halloween-Theme), itself inspired by [Christmas Theme](https://github.com/vcmikuu/ChristmasTheme) by laynia (vcmikuu) and CatsaCode. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for attribution and asset details.

Source code is licensed under **GNU GPLv3**; see [LICENSE](LICENSE). No installed custom-song recordings are redistributed.
