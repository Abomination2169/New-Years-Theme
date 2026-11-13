#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Collider.hpp"
#include "logging.hpp"

using namespace UnityEngine;

namespace NewYears::decor {
    static SafePtrUnity<GameObject> root;

    void Create() {
        if (root) return;
        // Diagnostic only: use the same SaberLogo anchor as the visible ball.
        auto logo = GameObject::Find("SaberLogo");
        if (!logo) {
            WARNING("New Year's diagnostic: SaberLogo not found; retry on menu activation");
            return;
        }
        root = GameObject::New_ctor("NewYearsVisibilityDiagnostic");
        if (!root) return;
        root->transform->SetParent(logo->transform, false);
        root->transform->set_localPosition(Vector3(0.0f, -0.6f, -0.25f));

        auto marker = GameObject::CreatePrimitive(PrimitiveType::Sphere);
        marker->set_name("BrightMagentaVisibilityMarker");
        marker->transform->SetParent(root->transform, false);
        marker->transform->set_localPosition(Vector3(0, 0, 0));
        marker->transform->set_localScale(Vector3(0.45f, 0.45f, 0.45f));
        auto collider = marker->GetComponent<Collider*>();
        if (collider) Object::Destroy(collider);
        auto renderer = marker->GetComponent<MeshRenderer*>();
        if (renderer) {
            auto mat = renderer->get_material();
            if (mat) {
                auto shader = Shader::Find("Sprites/Default");
                if (shader) mat->set_shader(shader);
                mat->set_color(Color(1.0f, 0.0f, 0.8f, 1.0f));
            }
        }
        const auto world = marker->transform->get_position();
        INFO("New Year's diagnostic marker world=(%.2f, %.2f, %.2f) active=%d",
             world.x, world.y, world.z, marker->get_activeInHierarchy());
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
