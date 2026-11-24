#include "main.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/MaterialPropertyBlock.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Time.hpp"
#include "custom-types/shared/register.hpp"
#include "c.hpp"
#include "logging.hpp"
#include "Confetti/confetti.hpp"
#include "Fireworks/fireworks.hpp"
#include "Audio/ambience.hpp"
#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/Object.hpp"
#include "System/Threading/Tasks/Tasks.hpp"
#include "custom-types/shared/delegate.hpp"
#include <functional>
#include <filesystem>
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

namespace {
    constexpr const char* floorDiagnosticBuild = "menu-floor-restoration-v3";
    SafePtrUnity<MeshRenderer> menuFloor;
    Color menuFloorBaseline{};
    bool hasMenuFloorBaseline = false;
    bool floorRestorationActive = false;
    bool loggedFloorRestore = false;

    MeshRenderer* FindMenuFloor(int colorProperty) {
        for (auto* renderer : Resources::FindObjectsOfTypeAll<MeshRenderer*>()) {
            if (!renderer || !renderer->get_enabled()) continue;
            auto go = renderer->get_gameObject();
            if (!go || !go->get_activeInHierarchy() || go->get_name() != "BasicMenuGround") continue;
            const auto materials = renderer->get_sharedMaterials();
            if (materials.size() != 1) continue;
            auto material = materials[0];
            if (material && material->get_name() == "MenuFloor" && material->HasProperty(colorProperty))
                return renderer;
        }
        return nullptr;
    }

}

// Unity copies this block into the renderer. Restore the caller's value after
// submission in case the same block is reused for other environment meshes.
MAKE_HOOK_MATCH(MenuFloorSetPropertyBlock,
    static_cast<void (Renderer::*)(MaterialPropertyBlock*)>(&Renderer::SetPropertyBlock),
    void, Renderer* self, MaterialPropertyBlock* block) {
    if (block && hasMenuFloorBaseline && floorRestorationActive && menuFloor &&
        self == static_cast<Renderer*>(menuFloor.ptr()) && getModConfig().EnableMidnightLighting.GetValue()) {
        static const int colorProperty = Shader::PropertyToID("_Color");
        const auto incoming = block->GetColor(colorProperty);
        // Ignore neutral and unset overrides, including the guard's own writes.
        if ((incoming.r != incoming.g || incoming.g != incoming.b) &&
            (incoming.r != menuFloorBaseline.r || incoming.g != menuFloorBaseline.g ||
             incoming.b != menuFloorBaseline.b || incoming.a != menuFloorBaseline.a)) {
            block->SetColor(colorProperty, menuFloorBaseline);
            MenuFloorSetPropertyBlock(self, block);
            block->SetColor(colorProperty, incoming);
            if (!loggedFloorRestore) {
                loggedFloorRestore = true;
                INFO("[MenuFloorRestoration] intercepted BasicMenuGround _Color before submission RGBA=({},{},{},{})",
                     incoming.r, incoming.g, incoming.b, incoming.a);
            }
            return;
        }
    }
    MenuFloorSetPropertyBlock(self, block);
}

DECLARE_CLASS_CODEGEN(NewYears, FloorRendererSample, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(float, nextLookupAt);
    DECLARE_INSTANCE_FIELD(UnityEngine::MaterialPropertyBlock*, propertyBlock);
    DECLARE_INSTANCE_METHOD(void, LateUpdate);
};
DEFINE_TYPE(NewYears, FloorRendererSample);

void NewYears::FloorRendererSample::LateUpdate() {
    if (UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore") {
        if (!floorRestorationActive) loggedFloorRestore = false;
        floorRestorationActive = true;
        return;
    }
    if (hasMenuFloorBaseline && (!floorRestorationActive || !getModConfig().EnableMidnightLighting.GetValue())) return;

    static const int colorProperty = Shader::PropertyToID("_Color");
    if (!menuFloor) {
        const float now = Time::get_realtimeSinceStartup();
        if (now < nextLookupAt) return;
        nextLookupAt = now + 0.5f;
        menuFloor = FindMenuFloor(colorProperty);
        if (!menuFloor) return;
    }

    auto* renderer = menuFloor.ptr();
    auto go = renderer->get_gameObject();
    if (!go || !go->get_activeInHierarchy() || !renderer->get_enabled()) return;
    renderer->GetPropertyBlock(propertyBlock);
    const auto current = propertyBlock->GetColor(colorProperty);

    if (!hasMenuFloorBaseline) {
        if (floorRestorationActive) return;
        // An unset override is reported as zero by the Quest binding.
        const bool unset = current.r == 0 && current.g == 0 && current.b == 0 && current.a == 0;
        menuFloorBaseline = unset ? renderer->get_sharedMaterials()[0]->GetColor(colorProperty) : current;
        hasMenuFloorBaseline = true;
        INFO("[MenuFloorRestoration] baselineRGBA=({},{},{},{})",
             menuFloorBaseline.r, menuFloorBaseline.g, menuFloorBaseline.b, menuFloorBaseline.a);
        return;
    }

    if (current.r == menuFloorBaseline.r && current.g == menuFloorBaseline.g &&
        current.b == menuFloorBaseline.b && current.a == menuFloorBaseline.a) return;
    // Preserve the other properties written by the menu lighting system.
    propertyBlock->SetColor(colorProperty, menuFloorBaseline);
    renderer->SetPropertyBlock(propertyBlock);
    if (!loggedFloorRestore) {
        loggedFloorRestore = true;
        INFO("[MenuFloorRestoration] LateUpdate corrected BasicMenuGround from RGBA=({},{},{},{})",
             current.r, current.g, current.b, current.a);
    }
}

namespace {
    void StartMenuFloorRestoration() {
        auto* probe = GameObject::New_ctor("NewYearsMenuFloorRestoration");
        auto* sampler = probe->AddComponent<NewYears::FloorRendererSample*>();
        if (!sampler) {
            GameObject::Destroy(probe);
            INFO("[MenuFloorRestoration] Failed to create floor controller");
            return;
        }
        sampler->nextLookupAt = Time::get_realtimeSinceStartup() + 0.5f;
        sampler->propertyBlock = MaterialPropertyBlock::New_ctor();
        GameObject::DontDestroyOnLoad(probe);
    }
}


static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
namespace NewYears::ball { void Create(); void Destroy(); }
static bool confettiNeedsPreviewRestore = false;
static bool ballNeedsPreviewRestore = false;
static void RestorePreviewDecor() {
    if (UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore") return;
    if (confettiNeedsPreviewRestore) {
        if (getModConfig().EnableConfetti.GetValue())
            NewYears::confetti::CreateConfetti();
        confettiNeedsPreviewRestore = false;
    }
    if (ballNeedsPreviewRestore) {
        NewYears::ball::Create();
        ballNeedsPreviewRestore = false;
    }
}

// Start menu music only after native initialization. Never substitute previews.
MAKE_HOOK_MATCH(NewYearsMenuMusicOnEnable, &SongPreviewPlayer::OnEnable,
                void, SongPreviewPlayer* self) {
    NewYearsMenuMusicOnEnable(self);
    if (getModConfig().EnableCelebrationAmbience.GetValue())
        NewYearsRememberMenuPlayer(self);
    RestorePreviewDecor();
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
    RestorePreviewDecor();
}

MAKE_HOOK_MATCH(NewYearsEnvironmentColors, &LightWithIdManager::SetColorForId,
                void, LightWithIdManager* self, int lightId, Color color) {
    if (UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore") {
        NewYears::fireworks::Destroy();
        NewYears::confetti::DestroyConfetti();
        confettiNeedsPreviewRestore = true;
        NewYears::ball::Destroy();
        ballNeedsPreviewRestore = true;
    }
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
    if (firstActivation) StartMenuFloorRestoration();
    if (getModConfig().EnableCelebrationAmbience.GetValue())
        NewYearsMainMenuActivated();
    if (getModConfig().EnableFireworks.GetValue())
        NewYears::fireworks::Create();
    NewYears::ball::Create();
    if (getModConfig().EnableConfetti.GetValue())
        NewYears::confetti::CreateConfetti();
    confettiNeedsPreviewRestore = false;
    ballNeedsPreviewRestore = false;
}

namespace {
    SafePtrUnity<GameObject> celebrationHost;
    SafePtrUnity<AudioClip> celebrationClip;
    bool celebrationActive = false;
    bool celebrationPlayed = false;
    bool celebrationLoading = false;
    constexpr const char* celebrationPath = "/sdcard/ModData/com.beatgames.beatsaber/Mods/NewYears/new_year_celebration.ogg";
    void StopCelebration() {
        celebrationActive = false;
        celebrationPlayed = false;
        if (celebrationHost) {
            auto* source = celebrationHost->GetComponent<AudioSource*>();
            if (source) source->Stop();
            Object::Destroy(celebrationHost.ptr());
            celebrationHost = nullptr;
        }
    }
    void PlayCelebration() {
        if (!celebrationActive || celebrationPlayed || !celebrationHost || !celebrationClip) return;
        auto* source = celebrationHost->GetComponent<AudioSource*>();
        if (!source) return;
        celebrationPlayed = true;
        source->set_playOnAwake(false);
        source->set_loop(false);
        source->set_spatialBlend(0.0f);
        source->set_volume(0.75f);
        source->set_clip(celebrationClip.ptr());
        source->Play();
    }
    void StartCelebration() {
        if (celebrationActive) return;
        celebrationActive = true;
        std::error_code ec;
        if (!std::filesystem::exists(celebrationPath, ec)) return;
        celebrationHost = GameObject::New_ctor("NewYearsCelebrationAudio");
        auto* source = celebrationHost->AddComponent<AudioSource*>();
        if (!source) { StopCelebration(); return; }
        source->set_playOnAwake(false);
        if (celebrationClip) { PlayCelebration(); return; }
        if (celebrationLoading) return;
        celebrationLoading = true;
        auto task = MediaAsyncLoader::New_ctor()->LoadAudioClipFromFilePathAsync(std::string(celebrationPath));
        task->GetAwaiter().OnCompleted(custom_types::MakeDelegate<System::Action*>(
            std::function([task]() {
                celebrationLoading = false;
                auto clip = task->get_Result();
                if (!clip) return;
                celebrationClip = clip.ptr();
                PlayCelebration();
            })));
    }
}
MAKE_HOOK_MATCH(NewYearsResultsActivate, &ResultsViewController::DidActivate, void,
    ResultsViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    NewYearsResultsActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    auto* completed = self ? self->__cordl_internal_get__levelCompletionResults() : nullptr;
    if (completed && completed->__cordl_internal_get_rank().value__ >= 5 &&
        getModConfig().EnableFireworks.GetValue()) StartCelebration();
    else StopCelebration();
}
MAKE_HOOK_MATCH(NewYearsResultsDeactivate, &ResultsViewController::DidDeactivate, void,
    ResultsViewController* self, bool removedFromHierarchy, bool screenSystemDisabling) {
    StopCelebration();
    NewYearsResultsDeactivate(self, removedFromHierarchy, screenSystemDisabling);
}

MOD_EXTERN_FUNC void setup(CModInfo* info) noexcept {
    *info = modInfo.to_c();
    getModConfig().Init(modInfo);
    Paper::Logger::RegisterFileContextId(PaperLogger.tag);
    INFO("New Year's Theme configured");
}
MOD_EXTERN_FUNC void late_load() noexcept {
    il2cpp_functions::Init();
    custom_types::Register::AutoRegister();
    INSTALL_HOOK(PaperLogger, NewYearsMenuMusicOnEnable);
    INSTALL_HOOK(PaperLogger, NewYearsMenuMusicCrossfade);
    INSTALL_HOOK(PaperLogger, NewYearsEnvironmentColors);
    INSTALL_HOOK(PaperLogger, MenuFloorSetPropertyBlock);
    INSTALL_HOOK(PaperLogger, NewYearsMenuActivate);
    INSTALL_HOOK(PaperLogger, NewYearsResultsActivate);
    INSTALL_HOOK(PaperLogger, NewYearsResultsDeactivate);
    INFO("New Year's Theme hooks installed");
}
