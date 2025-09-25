#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <PxPhysicsAPI.h>
#include "PhysicsOptions.hpp"

struct PhysicsComponent {
    float mass{1.0f};
    glm::vec3 velocity{0.0f,0.0f,0.0f};
    glm::vec3 acceleration{0.0f,0.0f,0.0f};
    glm::vec3 forces{0.0f,0.0f,0.0f};
    bool isStatic{false};

    physx::PxRigidActor* pxActor = nullptr;

    // optional creation parameters (set by builder / scene loader)
    std::shared_ptr<PhysicsOptions> options;

    PhysicsComponent() = default;
    PhysicsComponent(float m, const glm::vec3& vel = glm::vec3(0.0f), const glm::vec3& acc = glm::vec3(0.0f))
        : mass(m), velocity(vel), acceleration(acc) {}

    void AddForce(const glm::vec3& force) { forces += force; }
    void ClearForces() { forces = glm::vec3(0.0f); }
};
