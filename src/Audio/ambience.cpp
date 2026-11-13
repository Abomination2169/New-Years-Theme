#include "Audio/ambience.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "UnityEngine/Behaviour.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/SceneManagement.hpp"
#include "System/Threading/Tasks/Tasks.hpp"
#include "custom-types/shared/delegate.hpp"

#include <filesystem>
#include <algorithm>
#include <cctype>
#include <random>
#include <vector>
#include <string>
#include <functional>
#include <cmath>

namespace {
    namespace fs = std::filesystem;
    std::vector<std::string> tracks;
    size_t nextTrack = 0;
    bool scanned = false;
    bool loading = false;
    bool previewRequested = false;
    bool playbackStarted = false;
    SafePtrUnity<UnityEngine::AudioClip> menuClip;
    float menuStartRealtime = 0.0f;
    float savedMenuPosition = 0.0f;
    bool previewPlaying = false;
    SafePtrUnity<GlobalNamespace::SongPreviewPlayer> pendingMenuPlayer;
    bool mainMenuActivated = false;
    // SongCore and legacy SongLoader roots used on Beat Saber 1.40.8.
    const char* roots[] = {
        "/sdcard/ModData/com.beatgames.beatsaber/Mods/SongCore/CustomLevels",
        "/sdcard/ModData/com.beatgames.beatsaber/Mods/SongLoader/CustomLevels"
    };

    void ScanTracks() {
        if (scanned) return;
        scanned = true;
        for (auto* root : roots) {
            std::error_code ec;
            if (!fs::exists(root, ec)) continue;
            fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec), end;
            for (; it != end && !ec; it.increment(ec)) {
                if (!it->is_directory(ec)) continue;
                const auto dir = it->path();
                if (!fs::exists(dir / "Info.dat", ec) && !fs::exists(dir / "info.dat", ec)) continue;
                std::error_code innerEc;
                for (fs::directory_iterator songIt(dir, innerEc), songEnd; songIt != songEnd && !innerEc; songIt.increment(innerEc)) {
                    if (!songIt->is_regular_file(innerEc)) continue;
                    auto ext = songIt->path().extension().string();
                    for (char& ch : ext) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                    if (ext == ".ogg" || ext == ".egg") tracks.emplace_back(songIt->path().string());
                }
                it.disable_recursion_pending();
            }
        }
        std::mt19937 rng(std::random_device{}());
        std::shuffle(tracks.begin(), tracks.end(), rng);
        INFO("New Year's: found {} custom song audio files", tracks.size());
    }
}

void NewYearsRememberMenuPlayer(GlobalNamespace::SongPreviewPlayer* player) {
    pendingMenuPlayer = player;
}

void NewYearsMainMenuActivated() {
    mainMenuActivated = true;
    if (pendingMenuPlayer && !playbackStarted && !loading)
        PlayNewYearsCustomSong(pendingMenuPlayer.ptr());
}

bool NewYearsMenuMusicHasStarted() { return playbackStarted; }

void NewYearsMarkPreviewRequested() {
    previewRequested = true;
    if (menuClip && playbackStarted && !previewPlaying) {
        const float duration = menuClip->get_length();
        const float elapsed = UnityEngine::Time::get_realtimeSinceStartup() - menuStartRealtime;
        if (duration > 0.0f)
            savedMenuPosition = std::fmod(std::max(0.0f, elapsed), duration);
        previewPlaying = true;
        INFO("New Years: saved menu position {}", savedMenuPosition);
    }
}

float NewYearsGetMenuResumeTime(UnityEngine::AudioClip* clip, bool isDefault, float requestedTime) {
    if (isDefault && previewPlaying && menuClip && clip == menuClip.ptr()) {
        previewPlaying = false;
        menuStartRealtime = UnityEngine::Time::get_realtimeSinceStartup() - savedMenuPosition;
        INFO("New Years: resuming menu position {}", savedMenuPosition);
        return savedMenuPosition;
    }
    return requestedTime;
}

void PlayNewYearsCustomSong(GlobalNamespace::SongPreviewPlayer* player) {
    if (!player) return;
    ScanTracks();
    if (!mainMenuActivated || tracks.empty() || loading || playbackStarted) return; // Wait for the initialized menu.
    const std::string path = tracks[nextTrack++ % tracks.size()];
    loading = true;
    previewRequested = false;
    SafePtrUnity<GlobalNamespace::SongPreviewPlayer> safePlayer = player;
    INFO("New Years: loading custom menu track {}", path);
    auto task = GlobalNamespace::MediaAsyncLoader::New_ctor()->LoadAudioClipFromFilePathAsync(path);
    task->GetAwaiter().OnCompleted(custom_types::MakeDelegate<System::Action*>(
        std::function([safePlayer, task]() {
            loading = false;
            auto clip = task->get_Result();
            if (!safePlayer || !clip) {
                WARNING("New Years: custom menu clip load failed");
                return;
            }
            if (previewRequested || playbackStarted || !safePlayer->get_isActiveAndEnabled())
                return;
            if (UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore")
                return;
            playbackStarted = true;
            menuClip = clip;
            savedMenuPosition = 0.0f;
            previewPlaying = false;
            menuStartRealtime = UnityEngine::Time::get_realtimeSinceStartup();
            safePlayer->_defaultAudioClip = clip;
            INFO("New Years: custom menu clip ready; starting native crossfade");
            safePlayer->CrossfadeTo(clip, 1.0f, 0.0f, 0.5f, true, nullptr);
        })));
}
