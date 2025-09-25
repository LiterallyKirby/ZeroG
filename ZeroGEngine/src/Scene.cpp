#include "Scene.hpp"
#include "ECS.hpp"


Scene::Scene() {
    // top-right camera by default
    sceneCamera.position = {1.5f, 1.5f, 1.5f};
    sceneCamera.front = {-1.f, -1.f, -1.f};
    sceneCamera.up = {0.f, 1.f, 0.f};
    sceneCamera.aspect = 800.f / 600.f;



}

void Scene::Render() {
    const auto& ents = ECS::GetAllEntities();
    for (const auto& kv : ents) {
        const Entity& e = kv.second;
        auto t = ECS::GetTransform(e.id);
        if (!t) continue;
        CameraComponent* cam = nullptr;
        if (ECS::HasCamera(e.id)) cam = ECS::GetCamera(e.id);
        renderer.RenderEntity(e, *t, cam ? cam : &sceneCamera);
    }
}
