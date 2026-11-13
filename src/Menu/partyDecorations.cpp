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
        auto shader = Shader::Find("Sprites/Default");
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

    static void Bottle(Transform* parent, Vector3 at, float scale) {
        auto bottle = GameObject::New_ctor("ChampagneBottle");
        bottle->transform->SetParent(parent, false);
        bottle->transform->localPosition = at;
        bottle->transform->localScale = Vector3(scale, scale, scale);
        const Color green(0.045f, 0.26f, 0.18f, 1);
        const Color gold(0.95f, 0.72f, 0.24f, 1);
        Part(bottle->transform, "BottleBody", PrimitiveType::Cylinder,
             Vector3(0, 0.16f, 0), Vector3(0.17f, 0.32f, 0.17f), green);
        Part(bottle->transform, "BottleShoulder", PrimitiveType::Sphere,
             Vector3(0, 0.32f, 0), Vector3(0.16f, 0.13f, 0.16f), green);
        Part(bottle->transform, "BottleNeck", PrimitiveType::Cylinder,
             Vector3(0, 0.43f, 0), Vector3(0.068f, 0.19f, 0.068f), green);
        Part(bottle->transform, "GoldFoil", PrimitiveType::Cylinder,
             Vector3(0, 0.51f, 0), Vector3(0.077f, 0.095f, 0.077f), gold);
        Part(bottle->transform, "ChampagneLabel", PrimitiveType::Cylinder,
             Vector3(0, 0.20f, 0), Vector3(0.175f, 0.085f, 0.175f), gold);
    }

    static void TippedGlass(Transform* parent, Vector3 at, float angle) {
        auto glass = GameObject::New_ctor("KnockedOverChampagneGlass");
        glass->transform->SetParent(parent, false);
        glass->transform->localPosition = at;
        glass->transform->localRotation = Quaternion::Euler(Vector3(0, 0, angle));
        const Color silver(0.72f, 0.84f, 0.91f, 0.78f);
        Part(glass->transform, "FluteBowl", PrimitiveType::Cylinder,
             Vector3(0, 0.16f, 0), Vector3(0.10f, 0.25f, 0.10f), silver);
        Part(glass->transform, "FluteStem", PrimitiveType::Cylinder,
             Vector3(0, -0.045f, 0), Vector3(0.023f, 0.19f, 0.023f), silver);
        Part(glass->transform, "FluteFoot", PrimitiveType::Cylinder,
             Vector3(0, -0.15f, 0), Vector3(0.12f, 0.022f, 0.12f), silver);
    }

    static void PartyHat(Transform* parent, Vector3 at, Color color, float tilt) {
        auto hat = GameObject::New_ctor("NewYearsPartyHat");
        hat->transform->SetParent(parent, false);
        hat->transform->localPosition = at;
        hat->transform->localRotation = Quaternion::Euler(Vector3(0, 0, tilt));
        Part(hat->transform, "HatCone", PrimitiveType::Cylinder,
             Vector3(0, 0.19f, 0), Vector3(0.18f, 0.36f, 0.18f), color);
        Part(hat->transform, "HatBrim", PrimitiveType::Cylinder,
             Vector3(0, 0.015f, 0), Vector3(0.23f, 0.025f, 0.23f),
             Color(0.95f, 0.74f, 0.26f, 1));
        Part(hat->transform, "HatPom", PrimitiveType::Sphere,
             Vector3(0, 0.39f, 0), Vector3(0.095f, 0.095f, 0.095f),
             Color(1, 0.85f, 0.35f, 1));
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
        // Three modest groups arranged around the menu, relative to the existing logo.
        // Keep the center free for UI and the midnight ball.
        root->transform->SetParent(environment->transform, false);
        root->transform->localPosition = Vector3(0, 0, 0);
        const Vector3 spots[] = {
            Vector3(-7.55f, -0.65f, 13.25f),
            Vector3(7.95f, -0.50f, 5.40f),
            Vector3(3.25f, -0.65f, 14.60f)
        };
        const Color hats[] = {
            Color(0.20f, 0.70f, 1.0f, 1),
            Color(0.95f, 0.20f, 0.57f, 1),
            Color(0.53f, 0.34f, 0.97f, 1)
        };
        for (int i = 0; i < 3; ++i) {
            auto group = GameObject::New_ctor("NewYearsPartyCluster");
            group->transform->SetParent(root->transform, false);
            group->transform->localPosition = spots[i];
            Bottle(group->transform, Vector3(-0.30f, 0, 0), 0.75f);
            TippedGlass(group->transform, Vector3(0.28f, 0.10f, 0.06f), 77.0f);
            PartyHat(group->transform, Vector3(0.55f, 0, -0.15f), hats[i], i == 1 ? -12.0f : 8.0f);
        }
        INFO("New Year's: three floor-level party decoration clusters created");
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
