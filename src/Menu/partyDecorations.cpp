#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "logging.hpp"

using namespace UnityEngine;

namespace NewYears::decor {
    static SafePtrUnity<GameObject> root;

    static void Tint(GameObject* go, Color color) {
        if (!go) return;
        auto renderer = go->GetComponent<MeshRenderer*>();
        if (!renderer) return;
        auto material = renderer->get_material();
        if (!material) return;
        auto shader = Shader::Find("Sprites/Default");
        if (shader) material->set_shader(shader);
        material->set_color(color);
    }

    static GameObject* Part(Transform* parent, const char* name, PrimitiveType type,
                            Vector3 position, Vector3 scale, Color color,
                            Vector3 rotation = Vector3(0, 0, 0)) {
        auto go = GameObject::CreatePrimitive(type);
        if (!go) return nullptr;
        go->name = name;
        go->transform->SetParent(parent, false);
        go->transform->localPosition = position;
        go->transform->localScale = scale;
        go->transform->localRotation = Quaternion::Euler(rotation);
        Tint(go, color);
        return go;
    }

    // Small, layered geometry: no imported mesh dependencies and no update-loop cost.
    static void Bottle(Transform* parent, Vector3 at, float scale) {
        auto bottle = GameObject::New_ctor("DetailedChampagneBottle");
        bottle->transform->SetParent(parent, false);
        bottle->transform->localPosition = at;
        bottle->transform->localScale = Vector3(scale, scale, scale);
        const Color green(0.025f, 0.19f, 0.12f, 1);
        const Color greenHighlight(0.09f, 0.38f, 0.26f, 1);
        const Color gold(0.96f, 0.72f, 0.20f, 1);
        const Color cream(0.93f, 0.86f, 0.67f, 1);
        Part(bottle->transform, "ThickGlassBase", PrimitiveType::Cylinder,
             Vector3(0, 0.035f, 0), Vector3(0.195f, 0.065f, 0.195f), green);
        Part(bottle->transform, "DarkGlassBody", PrimitiveType::Cylinder,
             Vector3(0, 0.21f, 0), Vector3(0.18f, 0.35f, 0.18f), green);
        Part(bottle->transform, "CurvedShoulders", PrimitiveType::Sphere,
             Vector3(0, 0.385f, 0), Vector3(0.175f, 0.14f, 0.175f), green);
        Part(bottle->transform, "NarrowGlassNeck", PrimitiveType::Cylinder,
             Vector3(0, 0.49f, 0), Vector3(0.075f, 0.20f, 0.075f), green);
        Part(bottle->transform, "FoilNeck", PrimitiveType::Cylinder,
             Vector3(0, 0.535f, 0), Vector3(0.081f, 0.13f, 0.081f), gold);
        Part(bottle->transform, "FoilLip", PrimitiveType::Cylinder,
             Vector3(0, 0.603f, 0), Vector3(0.087f, 0.018f, 0.087f), cream);
        Part(bottle->transform, "FrontCreamLabel", PrimitiveType::Cube,
             Vector3(0, 0.235f, -0.177f), Vector3(0.23f, 0.155f, 0.012f), cream);
        Part(bottle->transform, "LabelGoldStripe", PrimitiveType::Cube,
             Vector3(0, 0.282f, -0.186f), Vector3(0.21f, 0.021f, 0.014f), gold);
        Part(bottle->transform, "LabelDarkStripe", PrimitiveType::Cube,
             Vector3(0, 0.21f, -0.187f), Vector3(0.16f, 0.027f, 0.014f), green);
        Part(bottle->transform, "GlassReflection", PrimitiveType::Cube,
             Vector3(-0.10f, 0.21f, -0.146f), Vector3(0.019f, 0.21f, 0.012f), greenHighlight);
    }

    static void TippedGlass(Transform* parent, Vector3 at, float angle) {
        auto glass = GameObject::New_ctor("KnockedOverWineGlass");
        glass->transform->SetParent(parent, false);
        glass->transform->localPosition = at;
        glass->transform->localRotation = Quaternion::Euler(Vector3(0, 0, angle));
        const Color clear(0.72f, 0.87f, 0.94f, 0.46f);
        const Color rim(0.89f, 0.95f, 1.0f, 0.78f);
        // Wide, rounded goblet bowl rather than the old cylindrical flute.
        Part(glass->transform, "RoundedWineBowl", PrimitiveType::Sphere,
             Vector3(0, 0.18f, 0), Vector3(0.29f, 0.29f, 0.29f), clear);
        Part(glass->transform, "WideOpenRim", PrimitiveType::Cylinder,
             Vector3(0, 0.326f, 0), Vector3(0.235f, 0.012f, 0.235f), rim);
        Part(glass->transform, "BowlLowerTaper", PrimitiveType::Sphere,
             Vector3(0, 0.048f, 0), Vector3(0.17f, 0.13f, 0.17f), clear);
        Part(glass->transform, "FineStem", PrimitiveType::Cylinder,
             Vector3(0, -0.105f, 0), Vector3(0.024f, 0.24f, 0.024f), rim);
        Part(glass->transform, "StemJoint", PrimitiveType::Sphere,
             Vector3(0, -0.22f, 0), Vector3(0.060f, 0.035f, 0.060f), rim);
        Part(glass->transform, "WideCircularFoot", PrimitiveType::Cylinder,
             Vector3(0, -0.245f, 0), Vector3(0.22f, 0.018f, 0.22f), clear);
        Part(glass->transform, "FootHighlight", PrimitiveType::Cylinder,
             Vector3(0, -0.231f, 0), Vector3(0.19f, 0.008f, 0.19f), rim);
    }

    static void PartyHat(Transform* parent, Vector3 at, Color color, float tilt) {
        auto hat = GameObject::New_ctor("DetailedNewYearsPartyHat");
        hat->transform->SetParent(parent, false);
        hat->transform->localPosition = at;
        hat->transform->localRotation = Quaternion::Euler(Vector3(0, 0, tilt));
        const Color gold(1.0f, 0.77f, 0.26f, 1);
        const Color light(0.97f, 0.91f, 0.72f, 1);
        // Three progressively narrower sections form a tapered party hat.
        Part(hat->transform, "WideConeBase", PrimitiveType::Cylinder,
             Vector3(0, 0.08f, 0), Vector3(0.29f, 0.15f, 0.29f), color);
        Part(hat->transform, "ConeMiddle", PrimitiveType::Cylinder,
             Vector3(0, 0.205f, 0), Vector3(0.20f, 0.14f, 0.20f), color);
        Part(hat->transform, "ConeTip", PrimitiveType::Sphere,
             Vector3(0, 0.31f, 0), Vector3(0.105f, 0.14f, 0.105f), color);
        Part(hat->transform, "MetallicBrim", PrimitiveType::Cylinder,
             Vector3(0, 0.012f, 0), Vector3(0.34f, 0.028f, 0.34f), gold);
        Part(hat->transform, "LowerGoldBand", PrimitiveType::Cylinder,
             Vector3(0, 0.075f, 0), Vector3(0.297f, 0.026f, 0.297f), light);
        Part(hat->transform, "UpperGoldBand", PrimitiveType::Cylinder,
             Vector3(0, 0.20f, 0), Vector3(0.213f, 0.018f, 0.213f), gold);
        Part(hat->transform, "MetallicPomPom", PrimitiveType::Sphere,
             Vector3(0, 0.405f, 0), Vector3(0.125f, 0.125f, 0.125f), gold);
        for (int i = 0; i < 6; ++i) {
            const float x = -0.105f + i * 0.042f;
            Part(hat->transform, "GlitterAccent", PrimitiveType::Sphere,
                 Vector3(x, 0.145f + (i % 2) * 0.055f, -0.135f),
                 Vector3(0.027f, 0.027f, 0.027f), light);
        }
    }

    void Create() {
        if (root) return;
        auto environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) {
            WARNING("New Year's: decorations waiting for DefaultMenuEnvironment");
            return;
        }
        root = GameObject::New_ctor("NewYearsPartyDecorations");
        if (!root) return;
        // Ground-level coordinates in the same DefaultMenuEnvironment local space
        // used by the proven Halloween and Thanksgiving floor decorations.
        // Slightly enlarged and moved closer so the objects are visible in VR.
        root->transform->SetParent(environment->transform, false);
        root->transform->localPosition = Vector3(0, 0, 0);
        const Vector3 spots[] = {
            Vector3(-4.6f, -0.65f, 7.5f),
            Vector3(4.8f, -0.65f, 7.0f),
            Vector3(0.8f, -0.65f, 10.5f)
        };
        // Restore the previously visible original three decoration positions.
        // Use the original wine-glass primitives, without bottles or hats.
        // The former 2x scale was oversized; 0.85x keeps these subtle.
        for (int i = 0; i < 3; ++i) {
            auto group = GameObject::New_ctor("OriginalSmallFallenWineGlass");
            group->transform->SetParent(root->transform, false);
            group->transform->localPosition = spots[i];
            group->transform->localScale = Vector3(0.85f, 0.85f, 0.85f);
            // The sideways bowl radius is approximately 0.145 in local space.
            // Raise its center to leave clearance above the floor baseline.
            TippedGlass(group->transform, Vector3(0.0f, 0.23f, 0.0f), 77.0f);
        }
        INFO("New Year's: three original wine glasses restored at visible ground locations");
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
