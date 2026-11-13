#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Color.hpp"
#include "GlobalNamespace/TubeBloomPrePassLight.hpp"
#include "GlobalNamespace/FlickeringNeonSign.hpp"
#include "logging.hpp"

namespace NewYears::logoColors {
void ApplyLogoColors() {
    const UnityEngine::Color white(1.0f, 1.0f, 1.0f, 1.0f);
    const UnityEngine::Color deepBlue(0.08f, 0.24f, 0.90f, 1.0f);

    const char* beatLights[] = {"BNeon", "ENeon", "ANeon", "TNeon"};
    const char* saberLights[] = {"SNeon", "ANeon2", "BNeon2", "ENeon2", "RNeon"};
    int changed = 0;
    for (const char* name : beatLights) {
        auto obj = UnityEngine::GameObject::Find(name);
        if (!obj) continue;
        auto neon = obj->GetComponent<GlobalNamespace::TubeBloomPrePassLight*>();
        if (neon) { neon->set_color(white); ++changed; }
    }
    for (const char* name : saberLights) {
        auto obj = UnityEngine::GameObject::Find(name);
        if (!obj) continue;
        auto neon = obj->GetComponent<GlobalNamespace::TubeBloomPrePassLight*>();
        if (neon) { neon->set_color(deepBlue); ++changed; }
    }
    if (auto obj = UnityEngine::GameObject::Find("EFlickering")) {
        auto flicker = obj->GetComponent<GlobalNamespace::FlickeringNeonSign*>();
        if (flicker) {
            flicker->__cordl_internal_set__spriteOnColor(white);
            flicker->__cordl_internal_set__lightOnColor(white);
        }
    }
    INFO("New Year's: white BEAT / deep blue SABER applied to %d neon objects", changed);
}
}
