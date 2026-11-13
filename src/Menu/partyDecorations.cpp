#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Collider.hpp"
#include "UnityEngine/Object.hpp"
#include "logging.hpp"

using namespace UnityEngine;

namespace NewYears::decor {
    static SafePtrUnity<GameObject> root;

    void Create() {
        if (root) return;
        auto environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) {
            WARNING("New Year's: snow waiting for DefaultMenuEnvironment");
            return;
        }
        root = GameObject::New_ctor("NewYearsIceBlueSnow");
        if (!root) return;
        root->transform->SetParent(environment->transform, false);
        root->transform->localPosition = Vector3(0, 0, 0);

        // The Christmas Theme's original ground-snow positions and sizes.
        // Christmas halves the positions to account for the Unity/Blender scale.
        struct SnowSpot { Vector3 position; Vector3 scale; };
        const SnowSpot spots[] = {
            {Vector3(-5.5f,-1.3f,16.1f), Vector3(10,2,10)},
            {Vector3(16.4f,-1.3f,0), Vector3(10,2,10)},
            {Vector3(6.5f,-1.3f,16.4f), Vector3(10,2,10)},
            {Vector3(15.9f,-1.0f,10.8f), Vector3(10,2,10)},
            {Vector3(-18.3f,-0.8f,-11.4f), Vector3(10,2,10)},
            {Vector3(-9.0f,-1.7f,-7.7f), Vector3(10,2,10)},
            {Vector3(-18.1f,-1.3f,2.7f), Vector3(10,2,10)},
            {Vector3(-2.5f,-1.3f,-14.2f), Vector3(10,2,10)},
            {Vector3(-15.7f,-1.3f,12.0f), Vector3(10,2,10)},
            {Vector3(10.0f,-1.3f,-10.5f), Vector3(10,2,10)},
            {Vector3(-15.1f,-1.3f,26.5f), Vector3(14.7f,3,14.7f)},
            {Vector3(25.4f,-0.9f,-15.5f), Vector3(14.7f,3.7f,14.7f)},
            {Vector3(7.0f,-1.3f,29.5f), Vector3(14.7f,3,14.7f)},
            {Vector3(29.9f,-1.3f,19.2f), Vector3(14.7f,3,14.7f)},
            {Vector3(34.4f,-1.3f,0.5f), Vector3(14.7f,3,14.7f)},
            {Vector3(-34.3f,-0.9f,16.6f), Vector3(14.7f,3.7f,14.7f)},
            {Vector3(-36.0f,-1.3f,-6.0f), Vector3(14.7f,3,14.7f)},
            {Vector3(-15.2f,-1.3f,-25.3f), Vector3(14.7f,3,14.7f)},
            {Vector3(7.9f,-1.3f,-30.2f), Vector3(14.7f,3,14.7f)},
            {Vector3(-25.8f,-0.7f,7.0f), Vector3(10,2,10)}
        };

        // Use a shared opaque material to avoid expensive per-pile instances.
        auto shader = Shader::Find("Standard");
        if (!shader) shader = Shader::Find("Sprites/Default");
        if (!shader) {
            WARNING("New Year's: snow shader unavailable");
            GameObject::Destroy(root.ptr());
            root = nullptr;
            return;
        }
        auto material = Material::New_ctor(shader);
        const Color iceBlue(0.53f, 0.78f, 0.98f, 1.0f);
        material->set_color(iceBlue);
        material->SetColor("_Color", iceBlue);
        material->SetFloat("_Glossiness", 0.12f);
        material->SetFloat("_Metallic", 0.0f);

        for (const auto& spot : spots) {
            auto pile = GameObject::CreatePrimitive(PrimitiveType::Sphere);
            if (!pile) continue;
            pile->set_name("IceBlueSnowDrift");
            pile->transform->SetParent(root->transform, false);
            pile->transform->set_localPosition(Vector3(
                spot.position.x / 2.0f,
                spot.position.y / 2.0f,
                spot.position.z / 2.0f));
            pile->transform->set_localScale(spot.scale);
            auto* renderer = pile->GetComponent<MeshRenderer*>();
            if (renderer) renderer->set_sharedMaterial(material);
            auto* collider = pile->GetComponent<Collider*>();
            if (collider) Object::Destroy(collider);
        }
        INFO("New Year's: Christmas-style ice-blue ground snow created");
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
