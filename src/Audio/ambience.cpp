#include "Audio/ambience.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/GameObject.hpp"
#include "System/Threading/Tasks/Tasks.hpp"
#include "custom-types/shared/delegate.hpp"
#include <functional>
#include <string>

using namespace UnityEngine;

namespace NewYears::ambience {
    static SafePtrUnity<GameObject> ambientGo;
    static SafePtrUnity<AudioSource> ambientSource;
    static const std::string ambiencePath =
        "/sdcard/ModData/com.beatgames.beatsaber/Mods/NewYears/Celebration_Ambience.wav";

    void Start() {
        if (ambientGo || !fileexists(ambiencePath)) return;
        ambientGo = GameObject::New_ctor("NewYearsCelebrationAmbience");
        ambientSource = ambientGo->AddComponent<AudioSource*>();
        if (!ambientSource) {
            GameObject::Destroy(ambientGo.ptr());
            ambientGo = nullptr;
            return;
        }
        // Independent 2D source: do not change Beat Saber's original music.
        ambientSource->set_playOnAwake(false);
        ambientSource->set_spatialBlend(0.0f);
        ambientSource->set_loop(true);
        ambientSource->set_volume(0.055f);

        auto task = GlobalNamespace::MediaAsyncLoader::New_ctor()
            ->LoadAudioClipFromFilePathAsync(ambiencePath);
        task->GetAwaiter().OnCompleted(
            custom_types::MakeDelegate<System::Action*>(std::function([task]() {
                if (!ambientSource) return;
                auto clip = task->get_Result();
                if (!clip) {
                    WARNING("New Year's: ambience clip did not load");
                    return;
                }
                ambientSource->set_clip(clip);
                ambientSource->Play();
                INFO("New Year's: ambience playing under stock menu music");
            }))
        );
    }
    void Stop() {
        if (ambientGo) {
            GameObject::Destroy(ambientGo.ptr());
            ambientGo = nullptr;
            ambientSource = nullptr;
        }
    }
}
