#include "Scene.hpp"
#include "ECS.hpp"

#include <iostream>
#include "SceneLoader.hpp" // <-- add this

Scene::Scene() {




}

void Scene::Render() {
    const auto& ents = ECS::GetAllEntities();

    CameraComponent* cam = nullptr;

    // find the first camera entity
    for (const auto& kv : ents) {
        const Entity& e = kv.second;
        if (ECS::HasCamera(e.id)) {
            cam = ECS::GetCamera(e.id);
            break;
        }
    }

    if (!cam) {
        std::cerr << "No camera found — cannot render scene.\n";
        return;
    }

    for (const auto& kv : ents) {
        const Entity& e = kv.second;
        auto t = ECS::GetTransform(e.id);
        if (!t) continue;

        renderer.RenderEntity(e, *t, cam);
    }
}

