#pragma once
#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "CameraComponent.hpp"
#include <unordered_map>
#include <optional>

namespace ECS {
    // containers
    EntityID CreateEntity(std::shared_ptr<Mesh> mesh, const std::string& tag = "");
    bool HasEntity(EntityID id);
    Entity* GetEntity(EntityID id);
void AddPhysics(EntityID id, const PhysicsComponent& p);
PhysicsComponent* GetPhysics(EntityID id);
bool HasPhysics(EntityID id);
    // transform
    void AddTransform(EntityID id, const TransformComponent& t);
    TransformComponent* GetTransform(EntityID id);
    bool HasTransform(EntityID id);
  std::string GetTag(EntityID id);
    void SetTag(EntityID id, const std::string& tag);
    // camera
    void AddCamera(EntityID id, const CameraComponent& c);
    CameraComponent* GetCamera(EntityID id);
    bool HasCamera(EntityID id);
void Clear();
    // iterate
    const std::unordered_map<EntityID, Entity>& GetAllEntities();

    // destroy
    void DestroyEntity(EntityID id);
}
