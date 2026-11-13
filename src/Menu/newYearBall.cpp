#include <cmath>
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "logging.hpp"

using namespace UnityEngine;

namespace NewYears::ball {
    static SafePtrUnity<GameObject> root;

    static void Colorize(GameObject* object, Color color) {
        if (!object) return;
        auto renderer = object->GetComponent<MeshRenderer*>();
        if (!renderer) return;
        auto material = renderer->get_material();
        if (!material) return;
        auto shader = Shader::Find("Sprites/Default");
        if (shader) material->set_shader(shader);
        material->set_color(color);
    }

    void Create() {
        if (root) return;
        auto logo = GameObject::Find("SaberLogo");
        if (!logo) {
            WARNING("New Year's: SaberLogo anchor not found");
            return;
        }
        // Position in the same world-space neighborhood as the real logo,
        // not at a fixed camera coordinate. Tune after first Quest screenshot.
        root = GameObject::New_ctor("NewYearsMidnightBall");
        const Vector3 logoPosition = logo->transform->position;
        root->transform->position = Vector3(logoPosition.x, logoPosition.y - 1.35f, logoPosition.z + 0.15f);
        root->transform->rotation = Quaternion::get_identity();

        auto shell = GameObject::CreatePrimitive(PrimitiveType::Sphere);
        shell->name = "MidnightBallShell";
        shell->transform->SetParent(root->transform, false);
        shell->transform->localScale = Vector3(0.68f, 0.68f, 0.68f);
        Colorize(shell, Color(0.23f, 0.35f, 0.57f, 1.0f));

        // Faceted highlight beads arranged around a genuinely 3D sphere.
        // Vary latitude, longitude, and material color to create depth and
        // parallax rather than a flat billboard beneath the Beat Saber logo.
        for (int lat = -3; lat <= 3; ++lat) {
            float phi = lat * 0.37f;
            float cp = cosf(phi), sp = sinf(phi);
            for (int lon = 0; lon < 16; ++lon) {
                float theta = lon * 0.3926990817f + (lat & 1) * 0.19634954f;
                auto bead = GameObject::CreatePrimitive(PrimitiveType::Sphere);
                bead->name = "MidnightBallFacet";
                bead->transform->SetParent(root->transform, false);
                bead->transform->localPosition = Vector3(0.352f * cp * cosf(theta),
                                                           0.352f * sp,
                                                           0.352f * cp * sinf(theta));
                float scale = ((lon + lat + 16) % 5 == 0) ? 0.073f : 0.052f;
                bead->transform->localScale = Vector3(scale, scale, scale);
                Colorize(bead, ((lon + lat + 16) % 3 == 0)
                    ? Color(1.0f, 0.78f, 0.30f, 1.0f)
                    : Color(0.83f, 0.91f, 1.0f, 1.0f));
            }
        }

        auto cap = GameObject::CreatePrimitive(PrimitiveType::Cylinder);
        cap->name = "MidnightBallTopCap";
        cap->transform->SetParent(root->transform, false);
        cap->transform->localPosition = Vector3(0, 0.39f, 0);
        cap->transform->localScale = Vector3(0.16f, 0.045f, 0.16f);
        Colorize(cap, Color(1.0f, 0.78f, 0.30f, 1.0f));
        INFO("New Year's: 3D midnight ball created below SaberLogo");
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
