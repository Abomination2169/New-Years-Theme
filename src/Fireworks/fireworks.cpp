#include "Fireworks/fireworks.hpp"
#include "Confetti/particleController.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/ParticleSystem.hpp"
#include "UnityEngine/ParticleSystemRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include <string>

using namespace UnityEngine;
using namespace Flair;

namespace NewYears::fireworks {
    // A menu-owned particle effect. Never modify Beat Saber's results
    // FireworksController; its success/failure logic is entirely native.
    static SafePtrUnity<GameObject> sparks[3];

    static bool IsMenuScene() {
        auto scene = UnityEngine::SceneManagement::SceneManager::GetActiveScene();
        const std::string name = static_cast<std::string>(scene.name);
        return name != "GameCore" && name != "EmptyTransition";
    }

    void Create() {
        if (!IsMenuScene()) return;
        if (sparks[0]) return;
        GameObject* dust = nullptr;
        for (auto* particle : Resources::FindObjectsOfTypeAll<ParticleSystem*>()) {
            if (particle && particle->name == "DustPS" &&
                particle->gameObject && particle->gameObject->activeInHierarchy) {
                dust = particle->gameObject;
                break;
            }
        }
        if (!dust) {
            WARNING("New Years: menu fireworks waiting for active DustPS");
            return;
        }

        constexpr float xPositions[3] = {-11.0f, 0.0f, 11.0f};
        constexpr float zPositions[3] = {14.0f, 19.0f, 14.0f};
        const Color colors[3] = {
            Color(1.0f, 0.68f, 0.20f, 1.0f),
            Color(0.40f, 0.76f, 1.0f, 1.0f),
            Color(1.0f, 0.38f, 0.67f, 1.0f)
        };
        for (int i = 0; i < 3; ++i) {
            auto* go = GameObject::Instantiate(dust);
            if (!go) continue;
            go->name = "NewYearsMenuFireworkSparks";
            go->transform->parent = nullptr;
            go->transform->position = Vector3(xPositions[i], 6.0f, zPositions[i]);
            go->transform->rotation = Quaternion::Euler(Vector3(0.0f, 0.0f, 0.0f));
            go->active = true;
            auto* ps = go->GetComponent<ParticleSystem*>();
            auto* renderer = go->GetComponent<ParticleSystemRenderer*>();
            if (!ps || !renderer) {
                GameObject::Destroy(go);
                continue;
            }
            ParticleController::MainModule::set_loop(ps, true);
            ParticleController::MainModule::set_startLifetime(ps, ParticleController::MinMaxCurve(1.0f, 2.0f));
            ParticleController::MainModule::set_startSpeed(ps, ParticleController::MinMaxCurve(2.5f, 6.5f));
            ParticleController::MainModule::set_startSize(ps, ParticleController::MinMaxCurve(0.045f, 0.095f));
            ParticleController::MainModule::set_gravityModifier(ps, -0.02f);
            ParticleController::MainModule::set_maxParticles(ps, 180);
            ParticleController::EmissionModule::set_enabled(ps, true);
            ParticleController::EmissionModule::set_rateOverTime(ps, 28.0f);
            ParticleController::ShapeModule::set_shapeType(ps, ParticleController::ShapeType::Sphere);
            ParticleController::ShapeModule::set_radius(ps, 0.3f);
            ParticleController::CollisionModule::set_enabled(ps, false);
            ParticleController::TextureSheetAnimationModule::set_enabled(ps, false);
            ParticleController::ColorOverLifetimeModule::set_enabled(ps, false);
            ParticleController::ColorBySpeedModule::set_enabled(ps, false);
            ParticleController::MinMaxGradient tint{};
            tint.mode = ParticleController::GradientMode::Color;
            tint.colorMin = colors[i];
            tint.colorMax = colors[i];
            ParticleController::MainModule::set_startColor(ps, tint);
            auto material = renderer->get_material();
            auto shader = Shader::Find("Sprites/Default");
            if (material && shader) {
                material->set_shader(shader);
                material->SetColor(StringW("_Color"), colors[i]);
            }
            sparks[i] = go;
        }
        INFO("New Years: independent menu spark emitters created");
    }

    void Destroy() {
        for (auto& go : sparks) {
            if (go) GameObject::Destroy(go.ptr());
            go = nullptr;
        }
    }

    void Forget() { Destroy(); }
}
