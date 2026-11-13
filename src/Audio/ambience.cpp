#include "Audio/ambience.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "System/Threading/Tasks/Tasks.hpp"
#include "custom-types/shared/delegate.hpp"
#include <functional>
#include <string>

// Same single-track menu music implementation used by Halloween Theme.
static const std::string menuSongPath =
    "/sdcard/ModData/com.beatgames.beatsaber/Mods/NewYears/Midnight_Synthwave_Tokyo.ogg";

void PlayNewYearsMenuSong(GlobalNamespace::SongPreviewPlayer* songPreviewPlayer) {
    if (!songPreviewPlayer || !fileexists(menuSongPath)) {
        INFO("New Years: menu track missing; keeping default menu audio");
        return;
    }
    auto loadSongTask = GlobalNamespace::MediaAsyncLoader::New_ctor()
        ->LoadAudioClipFromFilePathAsync(menuSongPath);
    if (!loadSongTask) return;
    loadSongTask->GetAwaiter().OnCompleted(
        custom_types::MakeDelegate<System::Action*>(
            std::function([songPreviewPlayer, loadSongTask]() {
                auto clip = loadSongTask->get_Result();
                if (songPreviewPlayer && clip) {
                    songPreviewPlayer->_defaultAudioClip = clip;
                    INFO("New Years: Morgan Alex menu track loaded");
                }
            })
        )
    );
}
