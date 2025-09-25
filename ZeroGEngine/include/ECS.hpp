#pragma once

#include "TransformComponent.hpp"
#include "PhysicsComponent.hpp"
#include "CameraComponent.hpp"
#include "Entity.hpp"
#include "glad/glad.h"


#include <unordered_map>
#include <optional>

namespace ECS {

    // Entity management
    EntityID CreateEntity(std::shared_ptr<Mesh> mesh = nullptr, const std::string& tag = "");
    bool HasEntity(EntityID id);
    Entity* GetEntity(EntityID id);
    void DestroyEntity(EntityID id);
    void Clear();
    
    // Component management
    void AddPhysics(EntityID id, const PhysicsComponent& p);
    PhysicsComponent* GetPhysics(EntityID id);
    bool HasPhysics(EntityID id);
    
    void AddTransform(EntityID id, const TransformComponent& t);
    TransformComponent* GetTransform(EntityID id);
    bool HasTransform(EntityID id);
    
    void AddCamera(EntityID id, const CameraComponent& c);
    CameraComponent* GetCamera(EntityID id);
    bool HasCamera(EntityID id);

    // Entity tagging
    std::string GetTag(EntityID id);
    void SetTag(EntityID id, const std::string& tag);

    // Iterating over all entities
    const std::unordered_map<EntityID, Entity>& GetAllEntities();

    // Component getter
    template<typename T>
    T* GetComponent(EntityID id);

    // Utility functions
    template<typename T>
    std::unordered_map<EntityID, T>& componentStorage();

}  // namespace ECS
