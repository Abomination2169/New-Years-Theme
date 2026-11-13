#include "Fireworks/fireworks.hpp"
#include "Confetti/particleController.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/ParticleSystem.hpp"
#include "UnityEngine/ParticleSystemRenderer.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"

using namespace UnityEngine;
using namespace Flair;

namespace NewYears::fireworks {
    SafePtrUnity<GameObject> emitters[3];

    void Create() {
        if (emitters[0]) return;
        SafePtrUnity<GameObject> dust;
        for (auto* particle : Resources::FindObjectsOfTypeAll<ParticleSystem*>()) {
            if (particle && particle->name == "DustPS") {
                dust = particle->gameObject;
                break;
            }
        }
        if (!dust) {
            WARNING("New Years fireworks: DustPS template unavailable");
            return;
        }
        constexpr Vector3 positions[] = {
            Vector3(-9.0f, 10.0f, 16.0f),
            Vector3(0.0f, 12.0f, 19.0f),
            Vector3(9.0f, 10.0f, 16.0f)
        };
        const Color colors[] = {
            Color(1.0f, 0.35f, 0.85f, 1.0f),
            Color(1.0f, 0.82f, 0.25f, 1.0f),
            Color(0.3f, 0.8f, 1.0f, 1.0f)
        };
        for (int i = 0; i < 3; ++i) {
            emitters[i] = GameObject::Instantiate(dust.ptr());
            if (!emitters[i]) continue;
            emitters[i]->name = "NewYearsFireworks";
            emitters[i]->transform->parent = nullptr;
            emitters[i]->transform->position = positions[i];
            emitters[i]->transform->rotation = Quaternion::get_identity();
            auto* ps = emitters[i]->GetComponent<ParticleSystem*>();
            if (!ps) {
                GameObject::Destroy(emitters[i].ptr());
                emitters[i] = nullptr;
                continue;
            }
            ParticleController::MainModule::set_loop(ps, true);
            ParticleController::MainModule::set_startLifetime(ps, ParticleController::MinMaxCurve(1.3f, 2.4f));
            ParticleController::MainModule::set_startSpeed(ps, ParticleController::MinMaxCurve(1.8f, 3.5f));
            ParticleController::MainModule::set_startSize(ps, ParticleController::MinMaxCurve(0.06f, 0.13f));
            ParticleController::MainModule::set_maxParticles(ps, 100);
            ParticleController::MainModule::set_gravityModifier(ps, ParticleController::MinMaxCurve(0.08f));
            ParticleController::MinMaxGradient tint{};
            tint.mode = ParticleController::GradientMode::Color;
            tint.colorMin = colors[i];
            tint.colorMax = colors[i];
            ParticleController::MainModule::set_startColor(ps, tint);
            ParticleController::ShapeModule::set_enabled(ps, true);
            ParticleController::ShapeModule::set_shapeType(ps, ParticleController::ShapeType::Sphere);
            ParticleController::ShapeModule::set_radius(ps, 0.12f);
            ParticleController::EmissionModule::set_enabled(ps, true);
            ParticleController::EmissionModule::set_rateOverTime(ps, ParticleController::MinMaxCurve(14.0f));
            ParticleController::CollisionModule::set_enabled(ps, false);
            ParticleController::TextureSheetAnimationModule::set_enabled(ps, false);
            emitters[i]->active = true;
            ps->Play();
        }
        INFO("New Years: continuous menu fireworks initialized");
    }

    void Destroy() {
        for (auto& emitter : emitters) {
            if (emitter) {
                GameObject::Destroy(emitter.ptr());
                emitter = nullptr;
            }
        }
    }
}
