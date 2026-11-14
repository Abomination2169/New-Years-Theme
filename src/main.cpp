#include "main.hpp"
#include "c.hpp"
#include "logging.hpp"
#include "Confetti/confetti.hpp"
#include "Fireworks/fireworks.hpp"
#include "Audio/ambience.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "UnityEngine/AudioClip.hpp"

#include "GlobalNamespace/LightWithIdManager.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/SceneManagement.hpp"
#include "UnityEngine/Color.hpp"
#include "custom-types/shared/macros.hpp"
#include "scotland2/shared/modloader.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
namespace NewYears::ball { void Create(); void Destroy(); }

static bool newYearsMenuEffectsNeedRestore = false;
static void RestoreNewYearsMenuEffects();

// Start menu music only after native initialization. Never substitute previews.
MAKE_HOOK_MATCH(NewYearsMenuMusicOnEnable, &SongPreviewPlayer::OnEnable,
                void, SongPreviewPlayer* self) {
    NewYearsMenuMusicOnEnable(self);
    if (getModConfig().EnableCelebrationAmbience.GetValue())
        NewYearsRememberMenuPlayer(self);
    // SongPreviewPlayer is also enabled when returning directly to song selection.
    // Only restore after gameplay has ended, never on the GameCore results screen.
    if (newYearsMenuEffectsNeedRestore &&
        UnityEngine::SceneManagement::SceneManager::GetActiveScene().name != "GameCore")
        RestoreNewYearsMenuEffects();
}

MAKE_HOOK_MATCH(NewYearsMenuMusicCrossfade,
                static_cast<void(SongPreviewPlayer::*)(AudioClip*, float, float, float, bool, System::Action*)>(&SongPreviewPlayer::CrossfadeTo),
                void, SongPreviewPlayer* self, AudioClip* clip, float volume, float startTime,
                float duration, bool isDefault, System::Action* callback) {
    // Startup crossfades can be non-default too; do not cancel the initial
    // asynchronous menu track before it has started playing.
    if (!isDefault && NewYearsMenuMusicHasStarted()) NewYearsMarkPreviewRequested();
    const float resumeTime = NewYearsGetMenuResumeTime(clip, isDefault, startTime);
    NewYearsMenuMusicCrossfade(self, clip, volume, resumeTime, duration, isDefault, callback);
    // The preview player may already be enabled across the gameplay transition.
    if (newYearsMenuEffectsNeedRestore &&
        UnityEngine::SceneManagement::SceneManager::GetActiveScene().name != "GameCore")
        RestoreNewYearsMenuEffects();
}

// Track the transition out of gameplay from a callback used by both the main
// menu and song-selection preview environment. Avoid changing the native
// fireworks controller while the results screen is active.
static bool newYearsMenuEffectsSuspendedForGameplay = false;
static void RestoreNewYearsMenuEffects() {
    if (UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore") return;
    if (getModConfig().EnableFireworks.GetValue())
        NewYears::fireworks::Create();
    NewYears::ball::Create();
    if (getModConfig().EnableConfetti.GetValue())
        NewYears::confetti::CreateConfetti();
    newYearsMenuEffectsNeedRestore = false;
}

MAKE_HOOK_MATCH(NewYearsEnvironmentColors, &LightWithIdManager::SetColorForId,
                void, LightWithIdManager* self, int lightId, Color color) {
    const bool inGameplay = UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore";
    if (inGameplay) {
        if (!newYearsMenuEffectsSuspendedForGameplay) {
            // Release our menu-owned particle objects when gameplay begins.
            // Restore the native controller's original enabled state before
            // results, so Beat Saber's own success/failure rules remain in charge.
            NewYears::fireworks::Destroy();
            NewYears::confetti::DestroyConfetti();
            NewYears::ball::Destroy();
            newYearsMenuEffectsSuspendedForGameplay = true;
            newYearsMenuEffectsNeedRestore = true;
        }
        NewYearsEnvironmentColors(self, lightId, color);
        return;
    }

    if (newYearsMenuEffectsSuspendedForGameplay)
        newYearsMenuEffectsSuspendedForGameplay = false;

    // This hook is invoked on the returning menu environment even when
    // MainMenuViewController::DidActivate is not called (preview-only return).
    // Recreate only once, never from inside GameCore/results.
    if (newYearsMenuEffectsNeedRestore)
        RestoreNewYearsMenuEffects();

    if (!getModConfig().EnableMidnightLighting.GetValue()) {
        NewYearsEnvironmentColors(self, lightId, color);
        return;
    }
    const Color navy(0.035f, 0.095f, 0.31f, color.a);
    const Color tint = navy;
    constexpr float blend = 0.52f;
    NewYearsEnvironmentColors(self, lightId,
        Color(color.r * (1 - blend) + tint.r * blend,
              color.g * (1 - blend) + tint.g * blend,
              color.b * (1 - blend) + tint.b * blend, color.a));
}

MAKE_HOOK_MATCH(NewYearsMenuActivate, &MainMenuViewController::DidActivate,
                void, MainMenuViewController* self,
                bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    NewYearsMenuActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    if (getModConfig().EnableCelebrationAmbience.GetValue())
        NewYearsMainMenuActivated();
    RestoreNewYearsMenuEffects();
}

MOD_EXTERN_FUNC void setup(CModInfo* info) noexcept {
    *info = modInfo.to_c();
    getModConfig().Init(modInfo);
    Paper::Logger::RegisterFileContextId(PaperLogger.tag);
    INFO("New Year's Theme configured");
}
MOD_EXTERN_FUNC void late_load() noexcept {
    il2cpp_functions::Init();
    INSTALL_HOOK(PaperLogger, NewYearsMenuMusicOnEnable);
    INSTALL_HOOK(PaperLogger, NewYearsMenuMusicCrossfade);
    INSTALL_HOOK(PaperLogger, NewYearsEnvironmentColors);
    INSTALL_HOOK(PaperLogger, NewYearsMenuActivate);
    INFO("New Year's Theme hooks installed");
}
