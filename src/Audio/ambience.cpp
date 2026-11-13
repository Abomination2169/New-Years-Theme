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
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>
#include <algorithm>
#include <cctype>

using namespace UnityEngine;

DECLARE_CLASS_CODEGEN(NewYears, AmbienceMenuTicker, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_METHOD(void, Update);
};
DEFINE_TYPE(NewYears, AmbienceMenuTicker);

namespace NewYears::ambience {
    static SafePtrUnity<GameObject> ambientGo;
    static SafePtrUnity<AudioSource> musicSource;
    static SafePtrUnity<AudioClip> musicClip;
    static bool musicLoading = false;
    static const std::string musicPath =
        "/sdcard/ModData/com.beatgames.beatsaber/Mods/NewYears/Midnight_Synthwave_Tokyo.ogg";
    static std::mutex completionMutex;
    static std::queue<std::function<void()>> completions;
    static bool active = false;
    static bool selected = false;
    static bool fallbackAttempted = false;
    static unsigned int generation = 0;
    static bool menuSession = false;

    // SongCore 1.40.8 plus legacy SongLoader installations.
    static std::vector<std::string> FindCustomTracks() {
        std::vector<std::string> tracks;
        const char* roots[] = {
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/SongCore/CustomLevels",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/SongLoader/CustomLevels"
        };
        for (const char* rootPath : roots) {
            DIR* rootDir = opendir(rootPath);
            if (!rootDir) continue;
            struct dirent* folder;
            while ((folder = readdir(rootDir)) != nullptr && tracks.size() < 4096) {
                if (folder->d_name[0] == '.') continue;
                std::string folderPath = std::string(rootPath) + "/" + folder->d_name;
                struct stat folderStat;
                if (stat(folderPath.c_str(), &folderStat) != 0 || !S_ISDIR(folderStat.st_mode)) continue;
                DIR* songDir = opendir(folderPath.c_str());
                if (!songDir) continue;
                struct dirent* file;
                while ((file = readdir(songDir)) != nullptr && tracks.size() < 4096) {
                    std::string name = file->d_name;
                    auto dot = name.find_last_of('.');
                    if (dot == std::string::npos) continue;
                    std::string ext = name.substr(dot);
                    std::transform(ext.begin(), ext.end(), ext.begin(),
                                   [](unsigned char ch) { return std::tolower(ch); });
                    if (ext != ".ogg" && ext != ".egg") continue;
                    std::string path = folderPath + "/" + name;
                    struct stat st;
                    if (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode) && st.st_size > 0)
                        tracks.push_back(path);
                }
                closedir(songDir);
            }
            closedir(rootDir);
        }
        return tracks;
    }

    static void LoadMusic(const std::string& path, bool fallback) {
        if (!menuSession || !musicSource) return;
        if (musicLoading || musicClip) return;
        if (!fileexists(path)) {
            if (!fallback && !fallbackAttempted) {
                fallbackAttempted = true;
                LoadMusic(musicPath, true);
            }
            return;
        }
        musicLoading = true;
        const unsigned int requestGeneration = generation;
        INFO("New Year's: loading %s music", fallback ? "Morgan Alex fallback" : "custom song");
        auto task = GlobalNamespace::MediaAsyncLoader::New_ctor()
            ->LoadAudioClipFromFilePathAsync(path);
        if (!task) {
            musicLoading = false;
            if (!fallback && !fallbackAttempted) {
                fallbackAttempted = true;
                LoadMusic(musicPath, true);
            }
            return;
        }
        task->GetAwaiter().OnCompleted(
            custom_types::MakeDelegate<System::Action*>(std::function([task, fallback, requestGeneration]() {
                std::lock_guard<std::mutex> lock(completionMutex);
                completions.emplace([task, fallback, requestGeneration]() {
                    if (requestGeneration != generation || !menuSession || !musicSource) return;
                    musicClip = task->get_Result();
                    musicLoading = false;
                    if (musicClip) {
                        INFO("New Year's: %s music ready",
                             fallback ? "Morgan Alex fallback" : "custom song");
                    } else {
                        WARNING("New Year's: selected audio could not be decoded");
                        if (!fallback && !fallbackAttempted) {
                            fallbackAttempted = true;
                            LoadMusic(musicPath, true);
                        }
                    }
                });
            }))
        );
    }


    void Tick() {
        std::queue<std::function<void()>> ready;
        {
            std::lock_guard<std::mutex> lock(completionMutex);
            std::swap(ready, completions);
        }
        if (!musicSource) return;
        while (!ready.empty()) { ready.front()(); ready.pop(); }
        const auto scene = SceneManagement::SceneManager::GetActiveScene();
        if (const_cast<SceneManagement::Scene&>(scene).get_name() == "GameCore") {
            Stop();
            return;
        }
        if (active && musicSource && musicClip && !musicSource->get_isPlaying()) {
            musicSource->set_clip(musicClip.ptr());
            musicSource->Play();
        }
    }

    void Start() {
        if (SceneManagement::SceneManager::GetActiveScene().name == "GameCore") return;
        if (!ambientGo || !musicSource) {
            ++generation;
            menuSession = false;
            selected = false;
            musicLoading = false;
            musicClip = nullptr;
            ambientGo = nullptr;
            musicSource = nullptr;
        }
        if (!ambientGo) {
            ambientGo = GameObject::New_ctor("NewYearsCelebrationAmbience");
            if (!ambientGo) return;
            // Scene-owned audio is destroyed by Unity on gameplay transition.
            musicSource = ambientGo->AddComponent<AudioSource*>();
            ambientGo->AddComponent<NewYears::AmbienceMenuTicker*>();
            if (!musicSource) {
                GameObject::Destroy(ambientGo.ptr());
                ambientGo = nullptr;
                return;
            }
            if (musicSource) {
                musicSource->set_playOnAwake(false);
                musicSource->set_spatialBlend(0.0f);
                musicSource->set_loop(true);
                musicSource->set_volume(0.25f);
            }
        }
        menuSession = true;
        active = true;
        if (!selected) {
            selected = true;
            auto tracks = FindCustomTracks();
            if (!tracks.empty()) {
                const auto choice = static_cast<size_t>(std::time(nullptr)) % tracks.size();
                INFO("New Year's: found %d custom audio tracks", static_cast<int>(tracks.size()));
                LoadMusic(tracks[choice], false);
            } else {
                INFO("New Year's: no custom tracks found; using Morgan Alex soundtrack");
                fallbackAttempted = true;
                LoadMusic(musicPath, true);
            }
        }

    }

    void Stop() {
        if (!menuSession) return;
        INFO("New Years: stopping menu-owned audio before gameplay");
        menuSession = false;
        ++generation;
        active = false;
        selected = false;
        musicLoading = false;
        if (musicSource) {
            musicSource->Stop();
            musicSource->set_clip(nullptr);
        }
        musicClip = nullptr;
    }
}

void NewYears::AmbienceMenuTicker::Update() { NewYears::ambience::Tick(); }
