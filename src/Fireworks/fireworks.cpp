#include "Fireworks/fireworks.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "GlobalNamespace/FireworksController.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Behaviour.hpp"

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
        if (enabledByMod) return;
        for (auto* candidate : Resources::FindObjectsOfTypeAll<FireworksController*>()) {
            if (!candidate || !candidate->gameObject || !candidate->gameObject->activeInHierarchy)
                continue;
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
        WARNING("New Years: no active-scene native FireworksController found; no replacement particles created");
    }

    void Destroy() {
        if (controller && enabledByMod)
            controller->set_enabled(previouslyEnabled);
        controller = nullptr;
        enabledByMod = false;
    }
}
