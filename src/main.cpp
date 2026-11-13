#include "main.hpp"
#include "c.hpp"
#include "logging.hpp"
#include "Confetti/confetti.hpp"
#include "Audio/ambience.hpp"

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
namespace NewYears::logoColors { void ApplyLogoColors(); }

MAKE_HOOK_MATCH(NewYearsEnvironmentColors, &LightWithIdManager::SetColorForId,
                void, LightWithIdManager* self, int lightId, Color color) {
    if (!getModConfig().EnableMidnightLighting.GetValue() ||
        UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore") {
        NewYearsEnvironmentColors(self, lightId, color);
        return;
    }
    const Color navy(0.012f, 0.045f, 0.19f, color.a);
    const Color gold(0.97f, 0.65f, 0.24f, color.a);
    const Color silver(0.62f, 0.76f, 0.97f, color.a);
    // A coherent midnight-blue wash across ALL environment lights.
    // Gold and silver stay as small accents on the Beat Saber neon sign.
    const Color tint = navy;
    const float blend = 0.94f;
    NewYearsEnvironmentColors(self, lightId,
        Color(color.r * (1 - blend) + tint.r * blend,
              color.g * (1 - blend) + tint.g * blend,
              color.b * (1 - blend) + tint.b * blend, color.a));
}

MAKE_HOOK_MATCH(NewYearsMenuActivate, &MainMenuViewController::DidActivate,
                void, MainMenuViewController* self,
                bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    NewYearsMenuActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    if (!firstActivation) return;
    if (getModConfig().EnableMidnightLighting.GetValue())
        NewYears::logoColors::ApplyLogoColors();
    if (getModConfig().EnableConfetti.GetValue())
        NewYears::confetti::CreateConfetti();
    if (getModConfig().EnableCelebrationAmbience.GetValue())
        NewYears::ambience::Start();
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
    INFO("New Year's Theme hooks installed");
}
