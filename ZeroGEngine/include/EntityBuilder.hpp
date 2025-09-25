#pragma once
#include "MeshType.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"
#include "CameraComponent.hpp"
#include "PhysicsComponent.hpp"
#include "PhysicsOptions.hpp"
#include <memory>
#include <optional>

class EntityBuilder {
    MeshType meshType{MeshType::Cube};
    TransformComponent transform;
    std::string tag;
    std::optional<PhysicsComponent> physics; // optional physics
    std::optional<CameraComponent> camera;

public:
    EntityBuilder& WithMesh(MeshType m) { meshType = m; return *this; }
    EntityBuilder& WithTag(const std::string& t) { tag = t; return *this; }
    EntityBuilder& WithPosition(float x, float y, float z=0.f) { transform.position = {x,y,z}; return *this; }
    EntityBuilder& WithRotation(float x, float y, float z) { transform.rotation = {x,y,z}; return *this; }
    EntityBuilder& WithScale(float x, float y, float z) { transform.scale = {x,y,z}; return *this; }

    EntityBuilder& WithPhysics(const PhysicsComponent& p) { physics = p; return *this; }

    // NEW: set physics creation options (convenience)
    EntityBuilder& WithPhysicsOptions(const PhysicsOptions& opts) {
        PhysicsComponent p;
        p.options = std::make_shared<PhysicsOptions>(opts);
        p.mass = opts.mass;
        p.isStatic = opts.isStatic;
        physics = p;
        return *this;
    }

    EntityBuilder& WithCamera(const CameraComponent& c) { camera = c; return *this; }

    EntityID Build() {
        auto mesh = std::make_shared<Mesh>(meshType);
        EntityID id = ECS::CreateEntity(mesh, tag);
        ECS::AddTransform(id, transform);

        if (physics.has_value()) ECS::AddPhysics(id, physics.value());
        if (camera.has_value()) ECS::AddCamera(id, camera.value());

        return id;
    }
};
