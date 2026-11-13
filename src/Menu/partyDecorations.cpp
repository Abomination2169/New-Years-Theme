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

    static void Bottle(Transform* parent, Vector3 at, float scale) {
        auto bottle = GameObject::New_ctor("RealisticChampagneBottle");
        bottle->transform->SetParent(parent, false);
        bottle->transform->localPosition = at;
        bottle->transform->localScale = Vector3(scale, scale, scale);
        const Color green(0.028f, 0.12f, 0.078f, 1);
        const Color foil(0.88f, 0.64f, 0.20f, 1);
        Lathe(bottle->transform, "SmoothBottleGlass", {
            {0.0f,0.10f},{0.012f,0.17f},{0.045f,0.185f},
            {0.12f,0.185f},{0.33f,0.185f},{0.385f,0.175f},
            {0.425f,0.135f},{0.458f,0.080f},{0.54f,0.075f},
            {0.59f,0.075f},{0.61f,0.084f},{0.62f,0.084f}
        }, green);
        Lathe(bottle->transform, "GoldFoilCapsule", {
            {0.50f,0.078f},{0.54f,0.079f},{0.61f,0.087f},{0.625f,0.087f}
        }, foil);
        Lathe(bottle->transform, "CreamPaperLabel", {
            {0.185f,0.187f},{0.32f,0.187f}
        }, Color(0.90f,0.82f,0.65f,1));
        Lathe(bottle->transform, "GoldLabelBorders", {
            {0.185f,0.19f},{0.195f,0.19f}
        }, foil);
        Lathe(bottle->transform, "UpperGoldLabelBorder", {
            {0.309f,0.19f},{0.319f,0.19f}
        }, foil);
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

    static void PartyHat(Transform* parent, Vector3 at, Color color, float tilt) {
        auto hat = GameObject::New_ctor("TaperedMetallicPartyHat");
        hat->transform->SetParent(parent, false);
        hat->transform->localPosition = at;
        hat->transform->localRotation = Quaternion::Euler(Vector3(0, 0, tilt));
        Lathe(hat->transform, "ContinuousConicalHat", {
            {0.0f,0.24f},{0.015f,0.235f},{0.10f,0.185f},
            {0.20f,0.125f},{0.31f,0.06f},{0.38f,0.004f}
        }, color);
        Lathe(hat->transform, "GoldTrim", {
            {0.005f,0.247f},{0.025f,0.242f}
        }, Color(0.98f,0.76f,0.25f,1));
        Lathe(hat->transform, "GoldDiagonalRibbon", {
            {0.16f,0.157f},{0.178f,0.148f}
        }, Color(0.97f,0.73f,0.20f,1));
        Part(hat->transform, "MetallicPom", PrimitiveType::Sphere,
             Vector3(0,0.405f,0),Vector3(0.07f,0.07f,0.07f),
             Color(1.0f,0.83f,0.35f,1));
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
        const Color hats[] = {
            Color(0.20f, 0.70f, 1.0f, 1),
            Color(0.95f, 0.20f, 0.57f, 1),
            Color(0.53f, 0.34f, 0.97f, 1)
        };
        for (int i = 0; i < 3; ++i) {
            auto group = GameObject::New_ctor("NewYearsPartyCluster");
            group->transform->SetParent(root->transform, false);
            group->transform->localPosition = spots[i];
            group->transform->localScale = Vector3(2.0f, 2.0f, 2.0f);
            Bottle(group->transform, Vector3(-0.30f, 0, 0), 0.75f);
            TippedGlass(group->transform, Vector3(0.28f, 0.10f, 0.06f), 77.0f);
            PartyHat(group->transform, Vector3(0.55f, 0, -0.15f), hats[i], i == 1 ? -12.0f : 8.0f);
        }
        INFO("New Year's: three floor-level party decoration clusters created");
    }

    void Destroy() {
        if (root) {
            GameObject::Destroy(root.ptr());
            root = nullptr;
        }
    }
}
