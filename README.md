# 🎆 New Year's Theme — Beat Saber Quest

Development preview for **Beat Saber 1.40.8_7379** on Quest 3.

Celebrate in a midnight-blue environment with gold and silver menu lighting, gently falling multicolored streamers, and a quiet ambient soundtrack layered beneath **Beat Saber's original menu music**.

## Features
- Standalone identity: `newyears`, `libnewyears.so`, `New-Years-Theme.qmod`
- Midnight navy environment-light tint with gold/silver accent lights
- Original Beat Saber neon sign retained, with gold BEAT and silver SABER
- One emitter adapted from Halloween Theme 1.40.8, with reduced density, slower gravity and world collision
- Original six-color confetti spritesheet: red, blue, green, purple, gold and silver
- Original low-volume synthesized prototype of distant party ambience and fireworks, played via a separate Unity AudioSource
- The QMOD contains no copy of Beat Saber's original menu music and does not replace its default audio clip

## Status
**Experimental development branch, not yet validated in-game.** GitHub Actions compiling and packaging successfully will not establish that lighting, streamers, collision or WAV ambience work correctly on the Quest. Please test before a public release or Mods Before Friday submission.

## Compatibility
Targets **Beat Saber 1.40.8_7379, Quest 3**. Do not enable this standalone preview alongside Halloween Theme until Holiday Theme Switcher coordinates theme activation, as both mods hook menu lighting.

## Credits and licensing
New Year's Theme is derived from [Halloween Theme](https://github.com/Abomination2169/Halloween-Theme), which derives from [Christmas Theme](https://github.com/vcmikuu/ChristmasTheme) by **laynia (vcmikuu)** and **CatsaCode**. The latter authors authorized GPLv3 reuse with proper attribution. They are not responsible for this derivative.

Covered source code: **GNU GPL-3.0-only**, see [LICENSE](LICENSE). Asset details and contributor credits: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

Developed by Abomination2169 with AI assistance. No commercial songs, fan remixes, or copyrighted countdown recordings are bundled.
