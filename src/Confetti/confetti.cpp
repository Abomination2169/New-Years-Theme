#include "Confetti/confetti.hpp"
#include "Confetti/particleController.hpp"
#include "main.hpp"
#include "assets.hpp"
#include "logging.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/ParticleSystemRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"

using namespace UnityEngine;
using namespace Flair;

namespace NewYears::confetti {
    SafePtrUnity<GameObject> templateDust;
    SafePtrUnity<GameObject> confettiGo;

    void CreateConfetti() {
        if (confettiGo) return;
        if (!templateDust) {
            for (auto particle : Resources::FindObjectsOfTypeAll<ParticleSystem*>()) {
                if (particle->name == "DustPS") {
                    templateDust = particle->gameObject;
                    break;
                }
            }
        }
        if (!templateDust) {
            WARNING("New Year's: DustPS template not found");
            return;
        }

        // Halloween 1.40.8's known-good SINGLE emitter and world collision.
        // Preserve the original dust object for other mods and theme switching.
        confettiGo = GameObject::Instantiate(templateDust->gameObject);
        confettiGo->name = "NewYearsConfettiPS";
        confettiGo->active = true;
        confettiGo->transform->parent = nullptr;
        confettiGo->transform->position = Vector3(0.0f, 7.0f, 0.0f);
        confettiGo->transform->rotation = Quaternion::Euler(Vector3(0.0f, 0.0f, 0.0f));

        auto* particles = confettiGo->GetComponent<ParticleSystem*>();
        auto* renderer = confettiGo->GetComponent<ParticleSystemRenderer*>();
        if (!particles || !renderer) {
            WARNING("New Year's: cloned particles/renderer unavailable");
            GameObject::Destroy(confettiGo.ptr());
            confettiGo = nullptr;
            return;
        }

        ParticleController::MainModule::set_startLifetime(particles, 16.0f);
        ParticleController::MainModule::set_startSize(particles, ParticleController::MinMaxCurve(0.10f, 0.30f));
        // Randomize the facing angle for each fleck; gently tumble as they fall.
        ParticleController::MainModule::set_startRotation(particles,
            ParticleController::MinMaxCurve(0.0f, 6.2831853f));
        ParticleController::RotationOverLifetimeModule::set_enabled(particles, true);
        ParticleController::RotationOverLifetimeModule::set_z(particles,
            ParticleController::MinMaxCurve(-0.55f, 0.55f));
        ParticleController::MainModule::set_maxParticles(particles, 100);
        ParticleController::MainModule::set_gravityModifier(particles, 0.008f);

        ParticleController::MinMaxGradient white{};
        white.mode = ParticleController::GradientMode::Color;
        white.colorMin = Color(1, 1, 1, 1);
        white.colorMax = Color(1, 1, 1, 1);
        ParticleController::MainModule::set_startColor(particles, white);
        ParticleController::ColorOverLifetimeModule::set_enabled(particles, false);
        ParticleController::ColorBySpeedModule::set_enabled(particles, false);
        ParticleController::EmissionModule::set_enabled(particles, true);
        ParticleController::EmissionModule::set_rateOverTime(particles, 5.5f);
        ParticleController::ShapeModule::set_shapeType(particles, ParticleController::ShapeType::Box);
        ParticleController::ShapeModule::set_position(particles, Vector3(0, 0, 0));
        ParticleController::ShapeModule::set_rotation(particles, Vector3(0, 0, 0));
        ParticleController::ShapeModule::set_scale(particles, Vector3(20.0f, 1.0f, 20.0f));

        ParticleController::CollisionModule::set_enabled(particles, true);
        ParticleController::CollisionModule::set_type(particles, ParticleController::CollisionType::World);
        ParticleController::CollisionModule::set_collidesWith(particles, LayerMask(0xffffffff));
        ParticleController::CollisionModule::set_dampen(particles, 0.08f);
        ParticleController::CollisionModule::set_bounce(particles, 0.0f);
        ParticleController::CollisionModule::set_lifetimeLoss(particles, 0.02f);

        auto texture = BSML::Utilities::LoadTextureRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::confetti_streamers_png));
        auto shader = Shader::Find("Sprites/Default");
        auto material = renderer->get_material();
        if (!shader || !material) {
            WARNING("New Year's: confetti shader unavailable");
            return;
        }
        material->set_shader(shader);
        material->SetTexture(StringW("_MainTex"), texture);
        material->SetColor(StringW("_Color"), Color(1, 1, 1, 1));

        constexpr int frames = 16;
        ParticleController::TextureSheetAnimationModule::set_enabled(particles, true);
        ParticleController::TextureSheetAnimationModule::set_mode(particles, ParticleController::AnimationMode::Grid);
        ParticleController::TextureSheetAnimationModule::set_numTilesX(particles, frames);
        ParticleController::TextureSheetAnimationModule::set_numTilesY(particles, 1);
        ParticleController::TextureSheetAnimationModule::set_frameOverTime(
            particles, ParticleController::MinMaxCurve(0.0f, 0.0f));
        ParticleController::TextureSheetAnimationModule::set_startFrame(
            particles, ParticleController::MinMaxCurve(0.0f, frames));
        ParticleController::TextureSheetAnimationModule::set_cycleCount(particles, 0);
    }

    void DestroyConfetti() {
        if (confettiGo) {
            GameObject::Destroy(confettiGo.ptr());
            confettiGo = nullptr;
        }
    }
}
