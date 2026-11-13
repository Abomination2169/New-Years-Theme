#include "main.hpp"
#include "logging.hpp"
#include "assets.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"

using namespace UnityEngine;

namespace NewYears::ball {
    static SafePtrUnity<GameObject> root;

    void Create() {
        if (root) return;
        auto logo = GameObject::Find("SaberLogo");
        if (!logo) {
            WARNING("New Year's: SaberLogo anchor not found");
            return;
        }

        // Raise the ball 0.30 Unity units to align its hanging cap with the B in SABER.
        // One transparent, textured quad replaces the 100+ primitive spheres.
        auto quad = GameObject::CreatePrimitive(PrimitiveType::Quad);
        if (!quad) return;
        quad->name = "NewYearsMidnightBall";
        const Vector3 p = logo->transform->position;
        quad->transform->position = Vector3(p.x - 0.30f, p.y - 3.85f, p.z + 0.15f);
        quad->transform->rotation = Quaternion::get_identity();
        quad->transform->localScale = Vector3(4.3f, 4.3f, 1.0f);

        auto renderer = quad->GetComponent<MeshRenderer*>();
        auto shader = Shader::Find("Sprites/Default");
        if (!renderer || !shader) {
            WARNING("New Year's: PNG billboard renderer/shader unavailable");
            GameObject::Destroy(quad);
            return;
        }
        auto material = renderer->get_material();
        if (!material) {
            GameObject::Destroy(quad);
            return;
        }
        auto texture = BSML::Utilities::LoadTextureRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::new_year_crystal_ball_png));
        if (!texture) {
            WARNING("New Year's: PNG billboard texture unavailable");
            GameObject::Destroy(quad);
            return;
        }
        material->set_shader(shader);
        material->SetTexture(StringW("_MainTex"), texture);
        material->SetColor(StringW("_Color"), Color(1, 1, 1, 1));
        root = quad;
        INFO("New Year's: transparent PNG ball billboard created");
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
