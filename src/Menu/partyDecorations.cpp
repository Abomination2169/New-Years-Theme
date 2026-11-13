#include <cmath>
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Collider.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "custom-types/shared/macros.hpp"
#include "logging.hpp"

using namespace UnityEngine;

namespace NewYears::decor { void UpdateSpotlight(); }

DECLARE_CLASS_CODEGEN(NewYears, GroundSpotlightTicker, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_METHOD(void, Update);
};
DEFINE_TYPE(NewYears, GroundSpotlightTicker);

namespace NewYears::decor {
    static SafePtrUnity<GameObject> root;
    static SafePtrUnity<GameObject> disk;
    static SafePtrUnity<Material> material;
    static float started = -1.0f;
    static int lastSpot = -1;

    // Preserve all twenty locations as a spatial pattern relative to a
    // verified, visible menu anchor, rather than an unverified scene parent.
    // Original Christmas snow positions, in environment-local coordinates.
    static constexpr float spots[20][3] = {
        {-5.5f, -1.3f, 16.1f},
        {16.4f, -1.3f, 0.0f},
        {6.5f, -1.3f, 16.4f},
        {15.9f, -1.0f, 10.8f},
        {-18.3f, -0.8f, -11.4f},
        {-9.0f, -1.7f, -7.7f},
        {-18.1f, -1.3f, 2.7f},
        {-2.5f, -1.3f, -14.2f},
        {-15.7f, -1.3f, 12.0f},
        {10.0f, -1.3f, -10.5f},
        {-15.1f, -1.3f, 26.5f},
        {25.4f, -0.9f, -15.5f},
        {7.0f, -1.3f, 29.5f},
        {29.9f, -1.3f, 19.2f},
        {34.4f, -1.3f, 0.5f},
        {-34.3f, -0.9f, 16.6f},
        {-36.0f, -1.3f, -6.0f},
        {-15.2f, -1.3f, -25.3f},
        {7.9f, -1.3f, -30.2f},
        {-25.8f, -0.7f, 7.0f}
    };

    void UpdateSpotlight() {
        if (!root || !disk || !material) return;
        if (SceneManagement::SceneManager::GetActiveScene().name == "GameCore") {
            disk->SetActive(false);
            return;
        }
        float now = Time::get_unscaledTime();
        if (started < 0.0f) started = now;
        constexpr float cycle = 2.8f;
        const float elapsed = now - started;
        const int step = static_cast<int>(elapsed / cycle);
        const int spot = step % 20;
        const float phase = elapsed - step * cycle;
        if (spot != lastSpot) {
            lastSpot = spot;
            disk->transform->set_localPosition(Vector3(
                spots[spot][0] / 2.0f,
                spots[spot][1] / 2.0f + 0.12f,
                spots[spot][2] / 2.0f));
            INFO("New Year's ground glow moved to position %d", spot + 1);
        }
        float fade = 0.0f;
        if (phase < 0.6f) fade = phase / 0.6f;
        else if (phase < 1.5f) fade = 1.0f;
        else if (phase < 2.1f) fade = (2.1f - phase) / 0.6f;
        material->set_color(Color(1.0f, 0.82f, 0.48f, 0.65f * fade));
        disk->SetActive(fade > 0.01f);
    }

    void Create() {
        if (root) return;
        auto environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) {
            WARNING("New Year's ground glow: DefaultMenuEnvironment unavailable");
            return;
        }
        auto shader = Shader::Find("Sprites/Default");
        if (!shader) {
            WARNING("New Year's ground glow: Sprites/Default unavailable");
            return;
        }
        root = GameObject::New_ctor("NewYearsSequentialWarmFloorGlow");
        if (!root) return;
        root->transform->SetParent(environment->transform, false);
        root->transform->set_localPosition(Vector3(0, 0, 0));
        auto surface = GameObject::CreatePrimitive(PrimitiveType::Cylinder);
        surface->set_name("WarmYellowFloorGlow");
        surface->transform->SetParent(root->transform, false);
        surface->transform->set_localScale(Vector3(2.8f, 0.012f, 2.8f));
        auto collider = surface->GetComponent<Collider*>();
        if (collider) Object::Destroy(collider);
        auto renderer = surface->GetComponent<MeshRenderer*>();
        if (renderer) {
            auto mat = renderer->get_material();
            if (mat) {
                mat->set_shader(shader);
                mat->set_color(Color(1.0f, 0.82f, 0.48f, 0.0f));
                material = mat;
            }
        }
        disk = surface;
        started = -1.0f;
        lastSpot = -1;
        root->AddComponent<NewYears::GroundSpotlightTicker*>();
        INFO("New Year's: Christmas-ground-anchored twenty-location floor glow initialized");
    }

    void Destroy() {
        disk = nullptr;
        material = nullptr;
        started = -1.0f;
        lastSpot = -1;
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}

void NewYears::GroundSpotlightTicker::Update() {
    NewYears::decor::UpdateSpotlight();
}
