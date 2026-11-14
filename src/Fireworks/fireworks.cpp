#include "Fireworks/fireworks.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "GlobalNamespace/FireworksController.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Behaviour.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include <string>

using namespace UnityEngine;
using namespace GlobalNamespace;

namespace NewYears::fireworks {
    // Only operate on a native fireworks controller already instantiated in
    // the active scene. Do not clone prefab assets or construct a controller:
    // the native controller requires injected pools and audio dependencies.
    SafePtrUnity<FireworksController> controller;
    bool previouslyEnabled = false;
    bool enabledByMod = false;

    void Create() {
        auto scene = UnityEngine::SceneManagement::SceneManager::GetActiveScene();
        const std::string sceneName = static_cast<std::string>(scene.name);
        if (sceneName == "GameCore") {
            INFO("New Years fireworks: skip activation in GameCore/results");
            return;
        }
        if (controller && controller->gameObject && controller->gameObject->activeInHierarchy) {
            INFO("New Years fireworks: retained controller enabled={} modOwned={}",
                 controller->get_enabled(), enabledByMod);
            if (enabledByMod) return;
        } else {
            controller = nullptr;
            enabledByMod = false;
        }
        INFO("New Years fireworks: scanning active controllers in scene={}", sceneName);
        for (auto* candidate : Resources::FindObjectsOfTypeAll<FireworksController*>()) {
            if (!candidate || !candidate->gameObject || !candidate->gameObject->activeInHierarchy)
                continue;
            INFO("New Years fireworks: candidate enabled={}", candidate->get_enabled());
            controller = candidate;
            previouslyEnabled = candidate->get_enabled();
            if (!previouslyEnabled) {
                candidate->set_enabled(true);
                enabledByMod = true;
                INFO("New Years: enabled existing native FireworksController");
            } else {
                INFO("New Years: native FireworksController already enabled");
            }
            return;
        }
        WARNING("New Years: no active FireworksController in scene={}; retry on next menu callback", sceneName);
    }

    void Destroy() {
        if (controller && enabledByMod)
            controller->set_enabled(previouslyEnabled);
        controller = nullptr;
        enabledByMod = false;
    }
    void Forget() {
        // Gameplay/results own the native controller. Keep its enabled state
        // untouched; just discard any menu-side reference to it.
        controller = nullptr;
        enabledByMod = false;
        previouslyEnabled = false;
    }

}
