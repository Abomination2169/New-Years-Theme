#pragma once
namespace UnityEngine { class AudioClip; }
namespace NewYears::ambience {
    void Start();
    void Stop();
    void SetPreviewActive(bool active);
    UnityEngine::AudioClip* GetReplacementClip();
}
