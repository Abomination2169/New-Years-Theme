#include <cmath>
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Light.hpp"
#include "UnityEngine/LightType.hpp"
#include "UnityEngine/LightShadows.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "custom-types/shared/macros.hpp"
#include "logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

using namespace UnityEngine;

namespace NewYears::decor { void UpdateSpotlight(); }

DECLARE_CLASS_CODEGEN(NewYears, GroundSpotlightTicker, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_METHOD(void, Update);
};
DEFINE_TYPE(NewYears, GroundSpotlightTicker);

namespace NewYears::decor {
    static SafePtrUnity<GameObject> root;
    static SafePtrUnity<Light> activeLight;
    static float sequenceStart = -1.0f;
    static int lastLocation = -1;

    // The same twenty ground locations used by the Christmas Theme snow.
    // Its Blender/Unity conversion halves the X/Z positions.
    static constexpr float spotPositions[20][3] = {
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
        if (!root || !activeLight) return;
        if (SceneManagement::SceneManager::GetActiveScene().name == "GameCore") {
            activeLight->set_intensity(0.0f);
            return;
        }
        const float now = Time::get_unscaledTime();
        if (sequenceStart < 0.0f) sequenceStart = now;
        // One spot at a time: 0.6s fade in, 0.9s glow, 0.6s fade out,
        // then a 0.7s dark pause. Twenty positions repeat in sequence.
        constexpr float cycle = 2.8f;
        const float elapsed = now - sequenceStart;
        const int step = static_cast<int>(elapsed / cycle);
        const int location = step % 20;
        const float phase = elapsed - step * cycle;
        if (location != lastLocation) {
            lastLocation = location;
            auto t = activeLight->get_transform();
            t->set_localPosition(Vector3(
                spotPositions[location][0] * 0.5f,
                spotPositions[location][1] * 0.5f + 2.5f,
                spotPositions[location][2] * 0.5f));
        }
        float brightness = 0.0f;
        if (phase < 0.6f) brightness = phase / 0.6f;
        else if (phase < 1.5f) brightness = 1.0f;
        else if (phase < 2.1f) brightness = (2.1f - phase) / 0.6f;
        activeLight->set_intensity(0.65f * brightness);
    }

    void Create() {
        if (root) return;
        auto environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) {
            WARNING("New Year's: rotating ground spotlight waiting for DefaultMenuEnvironment");
            return;
        }
        root = GameObject::New_ctor("NewYearsSequentialGroundSpotlight");
        if (!root) return;
        root->transform->SetParent(environment->transform, false);
        root->transform->localPosition = Vector3(0, 0, 0);

        auto lamp = GameObject::New_ctor("SoftYellowMovingFloorSpot");
        lamp->transform->SetParent(root->transform, false);
        lamp->transform->localRotation = Quaternion::Euler(Vector3(90, 0, 0));
        auto light = lamp->AddComponent<Light*>();
        if (light) {
            il2cpp_utils::RunMethod(light, "set_type", LightType::Spot);
            light->set_color(Color(1.0f, 0.80f, 0.47f, 1.0f));
            light->set_intensity(0.0f);
            il2cpp_utils::RunMethod(light, "set_range", 5.5f);
            il2cpp_utils::RunMethod(light, "set_spotAngle", 75.0f);
            il2cpp_utils::RunMethod(light, "set_shadows", LightShadows::None);
            activeLight = light;
        }
        sequenceStart = -1.0f;
        lastLocation = -1;
        root->AddComponent<NewYears::GroundSpotlightTicker*>();
        INFO("New Year's: sequential warm floor spotlight created for twenty positions");
    }

    void Destroy() {
        activeLight = nullptr;
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
