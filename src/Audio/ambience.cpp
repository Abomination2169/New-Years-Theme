#include "Audio/ambience.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "custom-types/shared/macros.hpp"
#include <mutex>
#include <queue>
#include "System/Threading/Tasks/Tasks.hpp"
#include "custom-types/shared/delegate.hpp"
#include <functional>
#include <string>

using namespace UnityEngine;

DECLARE_CLASS_CODEGEN(NewYears, AmbienceMenuTicker, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_METHOD(void, Update);
};
DEFINE_TYPE(NewYears, AmbienceMenuTicker);

namespace NewYears::ambience {
    static SafePtrUnity<GameObject> ambientGo;
    static SafePtrUnity<AudioSource> ambientSource;
    static SafePtrUnity<AudioClip> ambientClip;
    static SafePtrUnity<AudioSource> musicSource;
    static SafePtrUnity<AudioClip> musicClip;
    static bool musicLoading = false;
    static const std::string musicPath =
        "/sdcard/ModData/com.beatgames.beatsaber/Mods/NewYears/Midnight_Synthwave_Tokyo.ogg";
    static std::mutex completionMutex;
    static std::queue<std::function<void()>> completions;
    static bool active = false;
    static bool loading = false;
    static const std::string ambiencePath =
        "/sdcard/ModData/com.beatgames.beatsaber/Mods/NewYears/Celebration_Ambience.wav";

    void Tick() {
        std::queue<std::function<void()>> ready;
        {
            std::lock_guard<std::mutex> lock(completionMutex);
            std::swap(ready, completions);
        }
        while (!ready.empty()) { ready.front()(); ready.pop(); }
        if (!ambientSource) return;
        const auto scene = SceneManagement::SceneManager::GetActiveScene();
        if (const_cast<SceneManagement::Scene&>(scene).get_name() == "GameCore") {
            if (ambientSource->get_isPlaying()) ambientSource->Stop();
            if (musicSource && musicSource->get_isPlaying()) musicSource->Stop();
            active = false;
            return;
        }
        if (active && musicSource && musicClip && !musicSource->get_isPlaying()) {
            musicSource->set_clip(musicClip.ptr());
            musicSource->Play();
        }
        if (active && ambientClip && !ambientSource->get_isPlaying()) {
            ambientSource->set_clip(ambientClip.ptr());
            ambientSource->Play();
        }
    }

    void Start() {
        if (!fileexists(ambiencePath)) {
            WARNING("New Year's: packaged ambience WAV missing");
            return;
        }
        if (!ambientGo) {
            ambientGo = GameObject::New_ctor("NewYearsCelebrationAmbience");
            if (!ambientGo) return;
            Object::DontDestroyOnLoad(ambientGo.ptr());
            ambientSource = ambientGo->AddComponent<AudioSource*>();
            musicSource = ambientGo->AddComponent<AudioSource*>();
            ambientGo->AddComponent<NewYears::AmbienceMenuTicker*>();
            if (!ambientSource) {
                GameObject::Destroy(ambientGo.ptr());
                ambientGo = nullptr;
                return;
            }
            ambientSource->set_playOnAwake(false);
            ambientSource->set_spatialBlend(0.0f);
            ambientSource->set_loop(true);
            ambientSource->set_volume(0.025f);
            if (musicSource) {
                musicSource->set_playOnAwake(false);
                musicSource->set_spatialBlend(0.0f);
                musicSource->set_loop(true);
                musicSource->set_volume(0.25f);
            }
        }
        active = true;
        if (musicSource && !musicClip && !musicLoading && fileexists(musicPath)) {
            musicLoading = true;
            auto musicTask = GlobalNamespace::MediaAsyncLoader::New_ctor()
                ->LoadAudioClipFromFilePathAsync(musicPath);
            if (musicTask) {
                musicTask->GetAwaiter().OnCompleted(
                    custom_types::MakeDelegate<System::Action*>(std::function([musicTask]() {
                        std::lock_guard<std::mutex> lock(completionMutex);
                        completions.emplace([musicTask]() {
                            musicClip = musicTask->get_Result();
                            musicLoading = false;
                            if (!musicClip) WARNING("New Year's: synthwave music failed to load");
                            else INFO("New Year's: synthwave music loaded");
                        });
                    }))
                );
            } else musicLoading = false;
        }
        if (ambientClip || loading) return;
        loading = true;
        auto task = GlobalNamespace::MediaAsyncLoader::New_ctor()
            ->LoadAudioClipFromFilePathAsync(ambiencePath);
        if (!task) { loading = false; return; }
        task->GetAwaiter().OnCompleted(
            custom_types::MakeDelegate<System::Action*>(std::function([task]() {
                // Async completions may run off the Unity thread. Only enqueue here.
                std::lock_guard<std::mutex> lock(completionMutex);
                completions.emplace([task]() {
                    ambientClip = task->get_Result();
                    loading = false;
                    if (!ambientClip) WARNING("New Year's: ambience clip did not load");
                    else INFO("New Year's: celebration ambience loaded");
                });
            }))
        );
    }

    void Stop() {
        active = false;
        if (ambientSource && ambientSource->get_isPlaying()) ambientSource->Stop();
        if (musicSource && musicSource->get_isPlaying()) musicSource->Stop();
    }
}

void NewYears::AmbienceMenuTicker::Update() { NewYears::ambience::Tick(); }
