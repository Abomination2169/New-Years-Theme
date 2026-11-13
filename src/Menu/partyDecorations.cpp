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

namespace NewYears::decor {
    static SafePtrUnity<GameObject> root;

    static void Tint(GameObject* go, Color color) {
        if (!go) return;
        auto renderer = go->GetComponent<MeshRenderer*>();
        if (!renderer) return;
        auto material = renderer->get_material();
        if (!material) return;
        auto shader = Shader::Find("Standard");
        if (!shader) shader = Shader::Find("Sprites/Default");
        if (shader) material->set_shader(shader);
        material->set_color(color);
    }

    static GameObject* Part(Transform* parent, const char* name, PrimitiveType type,
                            Vector3 position, Vector3 scale, Color color,
                            Vector3 rotation = Vector3(0, 0, 0)) {
        auto go = GameObject::CreatePrimitive(type);
        if (!go) return nullptr;
        go->name = name;
        go->transform->SetParent(parent, false);
        go->transform->localPosition = position;
        go->transform->localScale = scale;
        go->transform->localRotation = Quaternion::Euler(rotation);
        Tint(go, color);
        return go;
    }

    static void WineGlass(Transform* parent, Vector3 at, float angle) {
        auto glass = GameObject::New_ctor("PrimitiveWineGlass");
        glass->transform->SetParent(parent, false);
        glass->transform->localPosition = at;
        glass->transform->localRotation = Quaternion::Euler(Vector3(0, 0, angle));
        const Color clear(0.70f, 0.86f, 0.98f, 1.0f);
        const Color rim(0.95f, 0.98f, 1.0f, 1.0f);
        // Wide, rounded goblet bowl rather than the old cylindrical flute.
        Part(glass->transform, "RoundedWineBowl", PrimitiveType::Sphere,
             Vector3(0, 0.18f, 0), Vector3(0.29f, 0.29f, 0.29f), clear);
        Part(glass->transform, "WideOpenRim", PrimitiveType::Cylinder,
             Vector3(0, 0.326f, 0), Vector3(0.235f, 0.012f, 0.235f), rim);
        Part(glass->transform, "BowlLowerTaper", PrimitiveType::Sphere,
             Vector3(0, 0.048f, 0), Vector3(0.17f, 0.13f, 0.17f), clear);
        Part(glass->transform, "FineStem", PrimitiveType::Cylinder,
             Vector3(0, -0.105f, 0), Vector3(0.024f, 0.24f, 0.024f), rim);
        Part(glass->transform, "StemJoint", PrimitiveType::Sphere,
             Vector3(0, -0.22f, 0), Vector3(0.060f, 0.035f, 0.060f), rim);
        Part(glass->transform, "WideCircularFoot", PrimitiveType::Cylinder,
             Vector3(0, -0.245f, 0), Vector3(0.22f, 0.018f, 0.22f), clear);
        Part(glass->transform, "FootHighlight", PrimitiveType::Cylinder,
             Vector3(0, -0.231f, 0), Vector3(0.19f, 0.008f, 0.19f), rim);
    }

    void Create() {
        if (root) return;
        auto environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) {
            WARNING("New Year's: decorations waiting for DefaultMenuEnvironment");
            return;
        }
        root = GameObject::New_ctor("NewYearsPartyDecorations");
        if (!root) return;
        // Ground-level coordinates in the same DefaultMenuEnvironment local space
        // used by the proven Halloween and Thanksgiving floor decorations.
        // Slightly enlarged and moved closer so the objects are visible in VR.
        root->transform->SetParent(environment->transform, false);
        root->transform->localPosition = Vector3(0, 0, 0);
        // Eight of the original twenty Christmas ground-decoration sites.
        // Use the same environment-local coordinate conversion as the snow.
        const Vector3 spots[] = {
            Vector3(-2.75f, -0.65f, 8.05f),
            Vector3(3.25f, -0.65f, 8.20f),
            Vector3(7.95f, -0.50f, 5.40f),
            Vector3(-4.50f, -0.85f, -3.85f),
            Vector3(-1.25f, -0.65f, -7.10f),
            Vector3(5.00f, -0.65f, -5.25f),
            Vector3(3.50f, -0.65f, 14.75f),
            Vector3(3.95f, -0.65f, -15.10f)
        };
        for (int i = 0; i < 8; ++i) {
            auto group = GameObject::New_ctor(i % 2 == 0 ? "UprightWineGlass" : "FallenWineGlass");
            group->transform->SetParent(root->transform, false);
            group->transform->localPosition = spots[i];
            group->transform->localScale = Vector3(0.80f, 0.80f, 0.80f);
            // Half upright, half on their sides. The original glass foot
            // extends 0.254 units below its origin; keep that above ground.
            const bool upright = (i % 2 == 0);
            WineGlass(group->transform,
                      Vector3(0, upright ? 0.28f : 0.24f, 0),
                      upright ? 0.0f : (i % 4 == 1 ? 77.0f : -77.0f));
        }
        INFO("New Year's: eight opaque primitive wine glasses created (four upright, four fallen)");
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
