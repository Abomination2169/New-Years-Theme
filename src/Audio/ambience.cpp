#include "Audio/ambience.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "custom-types/shared/macros.hpp"
#include "custom-types/shared/delegate.hpp"
#include "System/Threading/Tasks/Tasks.hpp"
#include <functional>
#include <mutex>
#include <queue>
#include <string>

using namespace UnityEngine;

DECLARE_CLASS_CODEGEN(NewYears, MenuMusicLoadTicker, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_METHOD(void, Update);
};
DEFINE_TYPE(NewYears, MenuMusicLoadTicker);

namespace NewYears::ambience {
    static SafePtrUnity<GameObject> tickerObject;
    static SafePtrUnity<AudioClip> replacementClip;
    static std::mutex completionMutex;
    static std::queue<std::function<void()>> completions;
    static bool loading = false;
    static bool pendingInitialPlayback = false;
    static const std::string musicPath =
        "/sdcard/ModData/com.beatgames.beatsaber/Mods/NewYears/Midnight_Synthwave_Tokyo.ogg";

    AudioClip* GetReplacementClip() {
        if (SceneManagement::SceneManager::GetActiveScene().name == "GameCore") return nullptr;
        return replacementClip ? replacementClip.ptr() : nullptr;
    }

    void Tick() {
        std::queue<std::function<void()>> ready;
        {
            std::lock_guard<std::mutex> lock(completionMutex);
            std::swap(ready, completions);
        }
        while (!ready.empty()) { ready.front()(); ready.pop(); }
        if (pendingInitialPlayback && replacementClip &&
            SceneManagement::SceneManager::GetActiveScene().name != "GameCore") {
            for (auto player : Resources::FindObjectsOfTypeAll<GlobalNamespace::SongPreviewPlayer*>()) {
                if (!player) continue;
                player->_defaultAudioClip = replacementClip.ptr();
                player->CrossfadeToDefault();
                pendingInitialPlayback = false;
                INFO("New Years: activated replacement via native preview player");
                break;
            }
        }
    }

    void Start() {
        if (SceneManagement::SceneManager::GetActiveScene().name == "GameCore") return;
        if (!tickerObject) {
            tickerObject = GameObject::New_ctor("NewYearsMenuMusicLoader");
            if (!tickerObject) return;
            Object::DontDestroyOnLoad(tickerObject.ptr());
            tickerObject->AddComponent<NewYears::MenuMusicLoadTicker*>();
        }
        if (replacementClip || loading || !fileexists(musicPath)) return;
        loading = true;
        auto task = GlobalNamespace::MediaAsyncLoader::New_ctor()
            ->LoadAudioClipFromFilePathAsync(musicPath);
        if (!task) {
            loading = false;
            return;
        }
        task->GetAwaiter().OnCompleted(
            custom_types::MakeDelegate<System::Action*>(std::function([task]() {
                std::lock_guard<std::mutex> lock(completionMutex);
                completions.emplace([task]() {
                    replacementClip = task->get_Result();
                    loading = false;
                    if (replacementClip) {
                        pendingInitialPlayback = true;
                        INFO("New Years: menu replacement clip loaded");
                    }
                    else WARNING("New Years: failed to load replacement clip");
                });
            }))
        );
    }

    void Stop() {
        // Beat Saber owns all menu AudioSources. There is nothing to stop here.
        // Keep the loaded clip alive for future menu sessions.
    }
    void SetPreviewActive(bool) {}
}

void NewYears::MenuMusicLoadTicker::Update() { NewYears::ambience::Tick(); }
