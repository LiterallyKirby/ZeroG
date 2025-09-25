#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <PxPhysicsAPI.h>
#include "Entity.hpp"
#include "PhysicsComponent.hpp"
#include "iostream"
using namespace physx;

class PhysicsSystem {
public:
    PhysicsSystem();
    ~PhysicsSystem();

    // Initialize PhysX; call once after creating object
    void Init();

    // Create a PhysX actor for the given entity/component (call when entity created)
    void CreatePhysXActor(Entity &entity, PhysicsComponent &physComp);

    // Remove an actor and clean up (call when entity is destroyed)
    void RemoveActor(EntityID entityId);

    // Fixed-step update (call from your fixed-update loop)
    void FixedUpdate(float timestep);

    // Optional: fallback non-PhysX integrator for simple use
    void Integrate(Entity &entity, PhysicsComponent &physicsComp, float deltaTime);

    // Check if an entity has a physics actor
    inline bool HasActor(EntityID entityId) const {
        bool found = entityToActor.find(entityId) != entityToActor.end();
        if (!found) {
            std::cout << "    DEBUG: Entity " << entityId << " not found in entityToActor map" << std::endl;
            std::cout << "    DEBUG: Map contains " << entityToActor.size() << " entries: ";
            for (const auto& pair : entityToActor) {
                std::cout << pair.first << " ";
            }
            std::cout << std::endl;
        }
        return found;
    }

    // Get the PhysX actor for an entity (returns nullptr if not found)
    PxRigidActor* GetActor(EntityID entityId) const {
        auto it = entityToActor.find(entityId);
        return (it != entityToActor.end()) ? it->second : nullptr;
    }

private:
    // PhysX core objects
    PxFoundation* foundation = nullptr;
    PxPhysics* physics = nullptr;
    PxCpuDispatcher* dispatcher = nullptr;
    PxScene* scene = nullptr;
    PxMaterial* material = nullptr;

    // Fast entity-to-actor mapping
    std::unordered_map<EntityID, PxRigidActor*> entityToActor;
    
    // Track only dynamic actors for efficient updates
    std::vector<EntityID> dynamicEntities;
    
    // Set of entities marked for removal (processed at start of next update)
    std::unordered_set<EntityID> pendingRemovals;

    // Internal helper methods
    void ProcessPendingRemovals();
    void CleanupActor(EntityID entityId, PxRigidActor* actor);
    
    // Prevent copying
    PhysicsSystem(const PhysicsSystem&) = delete;
    PhysicsSystem& operator=(const PhysicsSystem&) = delete;
};
