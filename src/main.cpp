#include "main.hpp"
#include "c.hpp"
#include "logging.hpp"
#include "Confetti/confetti.hpp"
#include "Audio/ambience.hpp"

#include "GlobalNamespace/LightWithIdManager.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/SceneManagement.hpp"
#include "UnityEngine/Color.hpp"
#include "custom-types/shared/macros.hpp"
#include "scotland2/shared/modloader.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
namespace NewYears::ball { void Create(); }

// Let Beat Saber play selected map previews without competing menu audio.
MAKE_HOOK_MATCH(NewYearsSongPreview, 
                static_cast<void(SongPreviewPlayer::*)(AudioClip*, float, float, float, bool, System::Action*)>(&SongPreviewPlayer::CrossfadeTo),
                void, SongPreviewPlayer* self, AudioClip* clip, float volume,
                float startTime, float duration, bool isDefault,
                System::Action* onFadeOutCallback) {
    if (getModConfig().EnableCelebrationAmbience.GetValue() &&
        UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "MenuCore") {
        NewYears::ambience::SetPreviewing(!isDefault && clip != nullptr);
    }
    NewYearsSongPreview(self, clip, volume, startTime, duration, isDefault, onFadeOutCallback);
}

// Suppress only the game's default menu music; resume our soundtrack instead.
MAKE_HOOK_MATCH(NewYearsDefaultMenuMusic, &SongPreviewPlayer::CrossfadeToDefault,
                void, SongPreviewPlayer* self) {
    if (getModConfig().EnableCelebrationAmbience.GetValue() &&
        UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "MenuCore") {
        NewYears::ambience::SetPreviewing(false);
        return;
    }
    NewYearsDefaultMenuMusic(self);
}

MAKE_HOOK_MATCH(NewYearsEnvironmentColors, &LightWithIdManager::SetColorForId,
                void, LightWithIdManager* self, int lightId, Color color) {
    if (!getModConfig().EnableMidnightLighting.GetValue() ||
        UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore") {
        NewYearsEnvironmentColors(self, lightId, color);
        return;
    }
    const Color navy(0.035f, 0.095f, 0.31f, color.a);
    // A coherent midnight-blue wash across ALL environment lights.
    // Gold and silver stay as small accents on the Beat Saber neon sign.
    const Color tint = navy;
    // Avoid crushing floor/near-field illumination to near-black.
    // Retain significantly more of the original light intensity.
    const float blend = 0.52f;
    NewYearsEnvironmentColors(self, lightId,
        Color(color.r * (1 - blend) + tint.r * blend,
              color.g * (1 - blend) + tint.g * blend,
              color.b * (1 - blend) + tint.b * blend, color.a));
}

MAKE_HOOK_MATCH(NewYearsMenuActivate, &MainMenuViewController::DidActivate,
                void, MainMenuViewController* self,
                bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    NewYearsMenuActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    if (getModConfig().EnableCelebrationAmbience.GetValue()) {
        NewYears::ambience::Start();
        for (auto preview : Resources::FindObjectsOfTypeAll<SongPreviewPlayer*>()) {
            if (preview) preview->FadeOut(1.0f);
        }
    }
    if (!firstActivation) return;
    NewYears::ball::Create();
    if (getModConfig().EnableConfetti.GetValue())
        NewYears::confetti::CreateConfetti();
}

MOD_EXTERN_FUNC void setup(CModInfo* info) noexcept {
    *info = modInfo.to_c();
    getModConfig().Init(modInfo);
    Paper::Logger::RegisterFileContextId(PaperLogger.tag);
    INFO("New Year's Theme configured");
}
MOD_EXTERN_FUNC void late_load() noexcept {
    il2cpp_functions::Init();
    INSTALL_HOOK(PaperLogger, NewYearsEnvironmentColors);
    INSTALL_HOOK(PaperLogger, NewYearsMenuActivate);
    INSTALL_HOOK(PaperLogger, NewYearsDefaultMenuMusic);
    INSTALL_HOOK(PaperLogger, NewYearsSongPreview);
    INFO("New Year's Theme hooks installed");
}
