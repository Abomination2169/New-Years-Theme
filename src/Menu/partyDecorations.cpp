#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Collider.hpp"
#include "logging.hpp"

using namespace UnityEngine;

namespace NewYears::decor {
    static SafePtrUnity<GameObject> root;
    static SafePtrUnity<Material> glassMaterial;
    static SafePtrUnity<Material> trimMaterial;

    static GameObject* Part(Transform* parent, const char* name, PrimitiveType type,
                            Vector3 position, Vector3 scale, Material* material,
                            Vector3 rotation = Vector3(0, 0, 0)) {
        auto go = GameObject::CreatePrimitive(type);
        if (!go) return nullptr;
        go->name = name;
        go->transform->SetParent(parent, false);
        go->transform->localPosition = position;
        go->transform->localScale = scale;
        go->transform->localRotation = Quaternion::Euler(rotation);
        auto renderer = go->GetComponent<MeshRenderer*>();
        if (renderer && material) renderer->set_sharedMaterial(material);
        auto collider = go->GetComponent<Collider*>();
        if (collider) Object::Destroy(collider);
        return go;
    }

    static void WineGlass(Transform* parent, Vector3 at, float angle) {
        auto glass = GameObject::New_ctor("PrimitiveWineGlass");
        glass->transform->SetParent(parent, false);
        glass->transform->localPosition = at;
        glass->transform->localRotation = Quaternion::Euler(Vector3(0, 0, angle));
        auto* clear = glassMaterial.ptr();
        auto* rim = trimMaterial.ptr();
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

    void Create() {
        if (root) return;
        auto environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) {
            WARNING("New Year's: decorations waiting for DefaultMenuEnvironment");
            return;
        }
        // Match Halloween tombstones: clone the known-rendering Note material.
        auto note = GameObject::Find("Note");
        Material* baseMaterial = nullptr;
        if (note) {
            auto renderer = note->GetComponent<MeshRenderer*>();
            if (renderer) baseMaterial = renderer->get_sharedMaterial();
        }
        if (!baseMaterial) {
            WARNING("New Year's: Note material unavailable; using primitive shader fallback");
            auto shader = Shader::Find("Sprites/Default");
            if (!shader) shader = Shader::Find("Standard");
            if (!shader) {
                WARNING("New Year's: no compatible material shader found; decorations not created");
                return;
            }
            baseMaterial = Material::New_ctor(shader);
        }
        glassMaterial = Object::Instantiate(baseMaterial);
        trimMaterial = Object::Instantiate(baseMaterial);
        glassMaterial->SetColor("_Color", Color(0.76f, 0.89f, 1.0f, 1.0f));
        trimMaterial->SetColor("_Color", Color(0.98f, 0.97f, 0.85f, 1.0f));
        root = GameObject::New_ctor("NewYearsPartyDecorations");
        if (!root) return;
        // Ground-level coordinates in the same DefaultMenuEnvironment local space
        // used by the proven Halloween and Thanksgiving floor decorations.
        // Slightly enlarged and moved closer so the objects are visible in VR.
        root->transform->SetParent(environment->transform, false);
        root->transform->localPosition = Vector3(0, 0, 0);
        INFO("New Year's: decoration root created under DefaultMenuEnvironment");
        // All twenty original Halloween tombstone positions, in the exact
        // DefaultMenuEnvironment local coordinates (original / 2).
        const Vector3 spots[] = {
            Vector3(-2.75f, -0.65f, 8.05f),
            Vector3(8.20f, -0.65f, 0.00f),
            Vector3(3.25f, -0.65f, 8.20f),
            Vector3(7.95f, -0.50f, 5.40f),
            Vector3(-9.15f, -0.40f, -5.70f),
            Vector3(-4.50f, -0.85f, -3.85f),
            Vector3(-9.05f, -0.65f, 1.35f),
            Vector3(-1.25f, -0.65f, -7.10f),
            Vector3(-7.85f, -0.65f, 6.00f),
            Vector3(5.00f, -0.65f, -5.25f),
            Vector3(-7.55f, -0.65f, 13.25f),
            Vector3(12.70f, -0.45f, -7.75f),
            Vector3(3.50f, -0.65f, 14.75f),
            Vector3(14.95f, -0.65f, 9.60f),
            Vector3(17.20f, -0.65f, 0.25f),
            Vector3(-17.15f, -0.45f, 8.30f),
            Vector3(-18.00f, -0.65f, -3.00f),
            Vector3(-7.60f, -0.65f, -12.65f),
            Vector3(3.95f, -0.65f, -15.10f),
            Vector3(-12.90f, -0.35f, 3.50f)
        };
        for (int i = 0; i < 20; ++i) {
            auto group = GameObject::New_ctor(i % 2 == 0 ? "UprightWineGlass" : "FallenWineGlass");
            group->transform->SetParent(root->transform, false);
            group->transform->set_localPosition(spots[i]);
            group->transform->set_localScale(Vector3(0.80f, 0.80f, 0.80f));
            const bool upright = (i % 2 == 0);
            WineGlass(group->transform,
                      Vector3(0, upright ? 0.28f : 0.24f, 0),
                      upright ? 0.0f : (i % 4 == 1 ? 77.0f : -77.0f));
        }
        INFO("New Year's: twenty wine glasses instantiated at Halloween tombstone sites (ten upright, ten fallen)");
        INFO("New Year's: decoration hierarchy active=%d", root->get_activeInHierarchy());
    }

    void Destroy() {
        glassMaterial = nullptr;
        trimMaterial = nullptr;
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
