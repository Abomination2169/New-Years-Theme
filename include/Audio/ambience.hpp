#pragma once
namespace GlobalNamespace { class SongPreviewPlayer; }
void PlayNewYearsCustomSong(GlobalNamespace::SongPreviewPlayer* player);
namespace UnityEngine { class AudioClip; }
void NewYearsMarkPreviewRequested();
float NewYearsGetMenuResumeTime(UnityEngine::AudioClip* clip, bool isDefault, float requestedTime);
