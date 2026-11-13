#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/SpriteRenderer.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "assets.hpp"
#include "logging.hpp"

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

        // The PNG includes the complete upper chrome hanging post, but no lower post.
        // Anchor the top of that post near the B in SABER instead of the logo center.
        // The sprite's native aspect ratio is preserved by uniform scaling.
        auto sprite = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::new_year_crystal_ball_png));
        if (!sprite) {
            WARNING("New Year's: crystal ball PNG failed to load");
            return;
        }

        root = GameObject::New_ctor("NewYearsCrystalBall");
        auto* renderer = root->AddComponent<SpriteRenderer*>();
        if (!renderer) {
            WARNING("New Year's: crystal ball SpriteRenderer unavailable");
            GameObject::Destroy(root.ptr());
            root = nullptr;
            return;
        }
        renderer->set_sprite(sprite);
        renderer->set_color(Color(1.0f, 1.0f, 1.0f, 1.0f));

        const Vector3 logoPosition = logo->transform->position;
        // The PNG is roughly square. At 100 pixels/unit, 0.34x yields
        // an approximately 4-metre-wide ornament; adjust after Quest test.
        constexpr float spriteScale = 0.34f;
        root->transform->localScale = Vector3(spriteScale, spriteScale, spriteScale);
        root->transform->rotation = Quaternion::get_identity();
        root->transform->position = Vector3(
            logoPosition.x - 0.75f,
            logoPosition.y - 1.95f,
            logoPosition.z + 0.15f);
        INFO("New Year's: large rainbow crystal ball PNG placed beneath SABER B");
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
