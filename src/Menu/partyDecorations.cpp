#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Light.hpp"
#include "UnityEngine/LightType.hpp"
#include "UnityEngine/LightShadows.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "logging.hpp"

using namespace UnityEngine;

namespace NewYears::decor {
    static SafePtrUnity<GameObject> root;

    void Create() {
        if (root) return;
        auto environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) {
            WARNING("New Year's: ground spotlights waiting for DefaultMenuEnvironment");
            return;
        }

        root = GameObject::New_ctor("NewYearsWarmGroundSpotlights");
        if (!root) return;
        root->transform->SetParent(environment->transform, false);
        root->transform->localPosition = Vector3(0, 0, 0);

        // Three restrained pools of warm light in the same proven menu areas.
        // Fixtures are above the floor, pointing straight down; no visible props.
        const Vector3 positions[] = {
            Vector3(-4.6f, 2.1f, 7.5f),
            Vector3(4.8f, 2.1f, 7.0f),
            Vector3(0.8f, 2.1f, 10.5f)
        };
        const Color warmYellow(1.0f, 0.80f, 0.47f, 1.0f);
        for (int i = 0; i < 3; ++i) {
            auto lamp = GameObject::New_ctor("SoftYellowFloorSpot");
            if (!lamp) continue;
            lamp->transform->SetParent(root->transform, false);
            lamp->transform->localPosition = positions[i];
            lamp->transform->localRotation = Quaternion::Euler(Vector3(90, 0, 0));
            auto light = lamp->AddComponent<Light*>();
            if (!light) continue;
            light->set_type(LightType::Spot);
            light->set_color(warmYellow);
            light->set_intensity(0.65f);
            light->set_range(5.5f);
            light->set_spotAngle(75.0f);
            light->set_shadows(LightShadows::None);
        }
        INFO("New Year's: three soft yellow downward floor spotlights created");
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
