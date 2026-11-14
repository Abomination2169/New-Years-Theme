# New Year's preview and results fireworks — implementation notes (test branch)

Goal: Restore New Year's fireworks and confetti immediately on return from gameplay to the song-selection/preview screen, without requiring Main Menu navigation. Preserve Beat Saber's native results-screen fireworks decision and timing.

## Current source findings
- Standalone `src/main.cpp`: `NewYearsEnvironmentColors` destroys fireworks, confetti and ball when `SceneManager::GetActiveScene().name == "GameCore"`. Only `MainMenuViewController::DidActivate` recreates them.
- Switcher `src/main.cpp`: `SeasonalMenuLighting` destroys fireworks and confetti in `GameCore`; `RefreshThemeDecor()` recreates them on menu activation, which can be skipped when returning directly to song selection.
- Switcher `src/NewYears/fireworks.cpp`: `Create()` finds an active `FireworksController` and enables it if disabled; `Destroy()` restores its previous enabled state. This can interfere with the controller's native results-screen lifecycle.

## Implementation acceptance criteria
1. Use a scene/flow transition callback that fires on gameplay exit **and** on return to the preview/song-selection screen, not only MainMenuViewController::DidActivate.
2. Only restore effects when the active theme is New Year's (or standalone settings enable them), the user is back in a menu scene, and valid scene-owned objects exist.
3. Make restore idempotent; if objects were destroyed during gameplay, detect stale pointers and recreate only once. Avoid duplicates and cross-scene references.
4. During gameplay and results, do not force-enable or disable the native `FireworksController` and do not override native results thresholds or trigger events.
5. Keep standalone and Switcher behavior aligned, preserving all existing audio, particle geometry, and original visuals.
6. Compile in GitHub Actions and test Quest 3: map → results (both celebration and no celebration) → song preview → main menu → map again, theme switching, and repeated loops.

## Status
Testing branches created; **no behavior change committed yet**. Exact Beat Saber 1.40.8 scene/flow callbacks and results controller lifecycle must be verified before editing hooks. Do not merge into stable source or v1.0.0 release until headset validation.
