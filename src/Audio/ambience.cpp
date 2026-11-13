#include "Audio/ambience.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
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

namespace {
    namespace fs = std::filesystem;
    std::vector<std::string> tracks;
    size_t nextTrack = 0;
    bool scanned = false;
    bool loading = false;
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

void PlayNewYearsCustomSong(GlobalNamespace::SongPreviewPlayer* player) {
    if (!player) return;
    ScanTracks();
    if (tracks.empty() || loading) return; // Keep stock music if unavailable.
    const std::string path = tracks[nextTrack++ % tracks.size()];
    loading = true;
    INFO("New Years: loading custom menu track {}", path);
    auto task = GlobalNamespace::MediaAsyncLoader::New_ctor()->LoadAudioClipFromFilePathAsync(path);
    task->GetAwaiter().OnCompleted(custom_types::MakeDelegate<System::Action*>(
        std::function([player, task]() {
            loading = false;
            auto clip = task->get_Result();
            if (!player || !clip) {
                WARNING("New Years: custom menu clip load failed");
                return;
            }
            if (UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore")
                return;
            player->_defaultAudioClip = clip;
            INFO("New Years: custom menu clip ready; starting native crossfade");
            player->CrossfadeTo(clip, 1.0f, 0.0f, 0.5f, true, nullptr);
        })));
}
