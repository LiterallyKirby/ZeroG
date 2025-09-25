#pragma once

#include "Mesh.hpp"
#include "PhysicsComponent.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include "CameraComponent.hpp"
#include "TransformComponent.hpp"
using EntityID = std::uint32_t;

struct Transform {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
};

struct Entity {
    EntityID id;
    std::string tag;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<PhysicsComponent> physics;
  TransformComponent transform; // <-- changed from Transform
    bool hasCamera = false;

std::shared_ptr<CameraComponent> camera; // optional camera storage
    Entity() = default;

    Entity(EntityID id_, std::shared_ptr<Mesh> m, const std::string& t = "")
        : id(id_), tag(t), mesh(std::move(m)) {}
  Entity& operator=(Entity&& other) noexcept {
        if (this != &other) {
            mesh = std::move(other.mesh);
            tag = std::move(other.tag);
        }
        return *this;
    }
    Entity(Entity&& other) noexcept = default;

    ~Entity() = default;
};
