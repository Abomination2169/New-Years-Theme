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
                indices.insert(indices.end(), {a, b, a + 1, a + 1, b, b + 1});
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
        auto shader = Shader::Find(transparent ? "Legacy Shaders/Transparent/Diffuse" : "Standard");
        if (!shader) shader = Shader::Find("Sprites/Default");
        if (shader) {
            auto material = Material::New_ctor(shader);
            material->set_color(color);
            renderer->set_sharedMaterial(material);
        }
        return go;
    }

    static void TippedGlass(Transform* parent, Vector3 at, float angle) {
        auto glass = GameObject::New_ctor("HollowWineGoblet");
        glass->transform->SetParent(parent, false);
        glass->transform->localPosition = at;
        glass->transform->localRotation = Quaternion::Euler(Vector3(0, 0, angle));
        const Color clear(0.78f,0.91f,0.98f,0.32f);
        const Color edge(0.86f,0.94f,1.0f,0.65f);
        Lathe(glass->transform, "HollowCurvedBowl", {
            {0.005f,0.035f},{0.03f,0.09f},{0.08f,0.15f},
            {0.15f,0.20f},{0.23f,0.235f},{0.31f,0.24f},
            {0.37f,0.225f},{0.395f,0.215f},
            {0.395f,0.207f},{0.37f,0.217f},{0.31f,0.232f},
            {0.23f,0.227f},{0.15f,0.192f},{0.08f,0.14f},
            {0.03f,0.078f},{0.005f,0.025f}
        }, clear, true);
        Lathe(glass->transform, "PolishedGlassLip", {
            {0.391f,0.214f},{0.395f,0.219f},{0.399f,0.214f}
        }, edge, true);
        Lathe(glass->transform, "ThinStemAndRoundFoot", {
            {-0.27f,0.17f},{-0.265f,0.18f},{-0.252f,0.16f},
            {-0.24f,0.045f},{-0.22f,0.022f},{-0.01f,0.022f},
            {0.015f,0.036f}
        }, edge, true);
    }

    void Create() {
        if (root) return;
        auto environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) {
            WARNING("New Year's: wine glasses waiting for DefaultMenuEnvironment");
            return;
        }
        root = GameObject::New_ctor("NewYearsGroundWineGlasses");
        if (!root) return;
        root->transform->SetParent(environment->transform, false);
        root->transform->localPosition = Vector3(0, 0, 0);

        // One small fallen goblet at each of three ground positions.
        // The third position is centered ahead of the player, away from the UI.
        // Ground baseline follows the previously visible menu decoration locations.
        constexpr float groundY = -0.65f;
        constexpr float glassScale = 0.55f;
        // A sideways goblet's widest bowl radius is 0.24 local units.
        // Center above the ground by the scaled radius plus a small clearance.
        constexpr float centerY = groundY + 0.24f * glassScale + 0.025f;
        const Vector3 positions[] = {
            Vector3(-3.8f, centerY, 7.0f),
            Vector3(3.8f, centerY, 7.0f),
            Vector3(0.0f, centerY, 5.0f)
        };
        const float rotations[] = {78.0f, 92.0f, 85.0f};
        for (int i = 0; i < 3; ++i) {
            auto group = GameObject::New_ctor("SmallFallenWineGlass");
            group->transform->SetParent(root->transform, false);
            group->transform->localPosition = positions[i];
            group->transform->localScale = Vector3(glassScale, glassScale, glassScale);
            TippedGlass(group->transform, Vector3(0, 0, 0), rotations[i]);
        }
        INFO("New Year's: three small ground-clearance wine glasses created");
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
