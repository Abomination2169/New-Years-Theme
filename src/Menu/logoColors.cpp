#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/SpriteRenderer.hpp"
#include "UnityEngine/Color.hpp"
#include "GlobalNamespace/TubeBloomPrePassLight.hpp"
#include "GlobalNamespace/FlickeringNeonSign.hpp"

namespace NewYears::logoColors {
void ApplyLogoColors() {
    const UnityEngine::Color gold(1.0f, 0.90f, 0.08f, 1.0f);
    const char* beatLights[] = {"BNeon", "ENeon", "ANeon", "TNeon"};
    for (const char* name : beatLights) {
        auto obj = UnityEngine::GameObject::Find(name);
        if (!obj) continue;
        auto* neon = obj->GetComponent<GlobalNamespace::TubeBloomPrePassLight*>();
        if (neon) neon->set_color(gold);
    }
    if (auto obj = UnityEngine::GameObject::Find("EFlickering")) {
        auto* flicker = obj->GetComponent<GlobalNamespace::FlickeringNeonSign*>();
        if (flicker) {
            flicker->__cordl_internal_set__spriteOnColor(gold);
            flicker->__cordl_internal_set__lightOnColor(gold);
        }
    }
}
}
