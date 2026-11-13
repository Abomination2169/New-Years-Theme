#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/MeshFilter.hpp"
#include "UnityEngine/Mesh.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include <vector>
#include <cmath>
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


    // Lathed, smooth-profile geometry replaces the old stacked primitive shapes.
    // Two-sided open goblet profile provides a real hollow bowl and visible rim.
    struct ProfilePoint { float y, radius; };
    static GameObject* Lathe(Transform* parent, const char* name,
                            const std::vector<ProfilePoint>& profile,
                            Color color, bool transparent = false) {
        constexpr int segments = 32;
        const int rings = static_cast<int>(profile.size());
        if (rings < 2) return nullptr;
        std::vector<Vector3> verts;
        std::vector<int> indices;
        verts.reserve(rings * (segments + 1));
        for (const auto& p : profile) {
            for (int j = 0; j <= segments; ++j) {
                const float a = 6.283185307f * j / segments;
                verts.emplace_back(p.radius * std::cos(a), p.y, p.radius * std::sin(a));
            }
        }
        for (int i = 0; i + 1 < rings; ++i) {
            for (int j = 0; j < segments; ++j) {
                int a = i * (segments + 1) + j;
                int b = a + segments + 1;
                indices.insert(indices.end(), {a, b, a + 1, a + 1, b, b + 1, a + 1, b, a, b + 1, b, a + 1});
            }
        }
        auto mesh = Mesh::New_ctor();
        mesh->set_vertices(il2cpp_utils::vectorToArray(verts));
        mesh->set_triangles(il2cpp_utils::vectorToArray(indices));
        mesh->RecalculateNormals();
        mesh->RecalculateBounds();
        auto go = GameObject::New_ctor(name);
        go->transform->SetParent(parent, false);
        auto filter = go->AddComponent<MeshFilter*>();
        filter->set_sharedMesh(mesh);
        auto renderer = go->AddComponent<MeshRenderer*>();
        auto shader = Shader::Find("Sprites/Default");
        if (!shader) shader = Shader::Find("Sprites/Default");
        if (shader) {
            auto material = Material::New_ctor(shader);
            material->set_color(color);
            renderer->set_sharedMaterial(material);
        }
        return go;
    }

    // Diagnostic visibility-first goblet. Use Unity's known-working primitive
    // meshes and renderer setup until the custom lathe rendering is verified.
    static void TippedGlass(Transform* parent, Vector3 at, float angle) {
        auto glass = GameObject::New_ctor("VisibleFallenWineGoblet");
        glass->transform->SetParent(parent, false);
        glass->transform->localPosition = at;
        glass->transform->localRotation = Quaternion::Euler(Vector3(0, 0, angle));
        const Color bowl(0.67f, 0.87f, 1.0f, 1.0f);
        const Color rim(0.93f, 0.98f, 1.0f, 1.0f);
        // Rounded wine bowl, not the narrow cylindrical flute.
        Part(glass->transform, "RoundedWineBowl", PrimitiveType::Sphere,
             Vector3(0, 0.19f, 0), Vector3(0.42f, 0.40f, 0.42f), bowl);
        Part(glass->transform, "GlassRim", PrimitiveType::Cylinder,
             Vector3(0, 0.38f, 0), Vector3(0.36f, 0.018f, 0.36f), rim);
        Part(glass->transform, "SlenderStem", PrimitiveType::Cylinder,
             Vector3(0, -0.09f, 0), Vector3(0.035f, 0.23f, 0.035f), rim);
        Part(glass->transform, "CircularFoot", PrimitiveType::Cylinder,
             Vector3(0, -0.23f, 0), Vector3(0.31f, 0.025f, 0.31f), rim);
    }

    void Create() {
        if (root) {
            auto* existing = root.ptr();
            if (existing && existing->get_activeInHierarchy()) {
                INFO("New Year's: existing wine-glass root is active");
                return;
            }
            WARNING("New Year's: stale or inactive wine-glass root; rebuilding");
            if (existing) GameObject::Destroy(existing);
            root = nullptr;
        }
        auto environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) {
            WARNING("New Year's: wine glasses waiting for DefaultMenuEnvironment");
            return;
        }
        root = GameObject::New_ctor("NewYearsGroundWineGlasses");
        if (!root) return;
        INFO("New Year's: menu environment active=%d", environment->get_activeInHierarchy());
        root->transform->SetParent(environment->transform, false);
        root->transform->localPosition = Vector3(0, 0, 0);

        // One small fallen goblet at each of three ground positions.
        // The third position is centered ahead of the player, away from the UI.
        // Ground baseline follows the previously visible menu decoration locations.
        constexpr float groundY = -0.65f;
        constexpr float glassScale = 1.05f;
        // A sideways goblet's widest bowl radius is 0.24 local units.
        // Center above the ground by the scaled radius plus a small clearance.
        constexpr float centerY = groundY + 0.24f * glassScale + 0.04f;
        const Vector3 positions[] = {
            Vector3(-4.6f, centerY, 7.5f),
            Vector3(4.8f, centerY, 7.0f),
            Vector3(0.0f, centerY, 7.8f)
        };
        const float rotations[] = {78.0f, 92.0f, 85.0f};
        for (int i = 0; i < 3; ++i) {
            auto group = GameObject::New_ctor("SmallFallenWineGlass");
            group->transform->SetParent(root->transform, false);
            group->transform->localPosition = positions[i];
            group->transform->localScale = Vector3(glassScale, glassScale, glassScale);
            TippedGlass(group->transform, Vector3(0, 0, 0), rotations[i]);
            const auto world = group->transform->get_position();
            INFO("New Year's: wine glass %d world=(%.2f, %.2f, %.2f) active=%d",
                 i, world.x, world.y, world.z, group->get_activeInHierarchy());
        }
        INFO("New Year's: three diagnostic wine goblets created; root active=%d", root->get_activeInHierarchy());
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
