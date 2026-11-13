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
    static SafePtrUnity<GameObject> glowDisk;
    static SafePtrUnity<Material> glowMaterial;
    static float sequenceStart = -1.0f;
    static int lastLocation = -1;

    // Original Christmas Theme's twenty ground positions, halved as in source.
    static constexpr float spots[20][3] = {
        {-5.5f,-1.3f,16.1f}, {16.4f,-1.3f,0.0f},
        {6.5f,-1.3f,16.4f}, {15.9f,-1.0f,10.8f},
        {-18.3f,-0.8f,-11.4f}, {-9.0f,-1.7f,-7.7f},
        {-18.1f,-1.3f,2.7f}, {-2.5f,-1.3f,-14.2f},
        {-15.7f,-1.3f,12.0f}, {10.0f,-1.3f,-10.5f},
        {-15.1f,-1.3f,26.5f}, {25.4f,-0.9f,-15.5f},
        {7.0f,-1.3f,29.5f}, {29.9f,-1.3f,19.2f},
        {34.4f,-1.3f,0.5f}, {-34.3f,-0.9f,16.6f},
        {-36.0f,-1.3f,-6.0f}, {-15.2f,-1.3f,-25.3f},
        {7.9f,-1.3f,-30.2f}, {-25.8f,-0.7f,7.0f}
    };

    void UpdateSpotlight() {
        if (!root || !glowDisk || !glowMaterial) return;
        if (SceneManagement::SceneManager::GetActiveScene().name == "GameCore") {
            glowDisk->SetActive(false);
            return;
        }
        const float now = Time::get_unscaledTime();
        if (sequenceStart < 0.0f) sequenceStart = now;
        constexpr float cycle = 2.8f;
        const float elapsed = now - sequenceStart;
        const int step = static_cast<int>(elapsed / cycle);
        const int location = step % 20;
        const float phase = elapsed - step * cycle;
        if (location != lastLocation) {
            lastLocation = location;
            glowDisk->transform->set_localPosition(Vector3(
                spots[location][0] * 0.5f,
                // Place just above the Christmas snow ground baseline.
                spots[location][1] * 0.5f + 0.11f,
                spots[location][2] * 0.5f));
        }
        float fade = 0.0f;
        if (phase < 0.6f) fade = phase / 0.6f;
        else if (phase < 1.5f) fade = 1.0f;
        else if (phase < 2.1f) fade = (2.1f - phase) / 0.6f;
        // Rendered geometry is visible even when the menu ignores dynamic lights.
        glowMaterial->set_color(Color(1.0f, 0.81f, 0.44f, fade * 0.36f));
        glowDisk->SetActive(fade > 0.005f);
    }

    void Create() {
        if (root) return;
        auto environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) {
            WARNING("New Year's: ground glow waiting for DefaultMenuEnvironment");
            return;
        }
        auto shader = Shader::Find("Sprites/Default");
        if (!shader) {
            WARNING("New Year's: soft glow shader not found");
            return;
        }
        root = GameObject::New_ctor("NewYearsSequentialGroundGlow");
        if (!root) return;
        root->transform->SetParent(environment->transform, false);
        root->transform->set_localPosition(Vector3(0, 0, 0));
        // Flat horizontal semi-transparent disk: a glow impression, not a true light.
        auto disk = GameObject::CreatePrimitive(PrimitiveType::Cylinder);
        disk->set_name("WarmFloorGlowDisk");
        disk->transform->SetParent(root->transform, false);
        disk->transform->set_localScale(Vector3(2.9f, 0.008f, 2.9f));
        auto collider = disk->GetComponent<Collider*>();
        if (collider) Object::Destroy(collider);
        auto renderer = disk->GetComponent<MeshRenderer*>();
        if (renderer) {
            auto mat = Material::New_ctor(shader);
            mat->set_color(Color(1.0f, 0.81f, 0.44f, 0.0f));
            renderer->set_sharedMaterial(mat);
            glowMaterial = mat;
        }
        glowDisk = disk;
        sequenceStart = -1.0f;
        lastLocation = -1;
        root->AddComponent<NewYears::GroundSpotlightTicker*>();
        INFO("New Year's: twenty-position animated warm ground glow created");
    }

    void Destroy() {
        glowDisk = nullptr;
        glowMaterial = nullptr;
        sequenceStart = -1.0f;
        lastLocation = -1;
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}

void NewYears::GroundSpotlightTicker::Update() {
    NewYears::decor::UpdateSpotlight();
}
