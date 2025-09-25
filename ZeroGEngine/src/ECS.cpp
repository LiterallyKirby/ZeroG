#include "ECS.hpp"

// Static storage for each component type
template <typename T>
std::unordered_map<EntityID, T>& ECS::componentStorage() {
    static std::unordered_map<EntityID, T> storage;
    return storage;
}

// Entity creation

EntityID ECS::CreateEntity(std::shared_ptr<Mesh> mesh, const std::string& tag) {
    static EntityID nextID = 0;
    EntityID newID = nextID++;
    Entity newEntity;
    newEntity.mesh = mesh;
    newEntity.tag = tag;

    // Register the entity in the global entity map using move semantics
    componentStorage<Entity>()[newID] = std::move(newEntity);

    return newID;
}


// Entity existence check
bool ECS::HasEntity(EntityID id) {
    return componentStorage<Entity>().find(id) != componentStorage<Entity>().end();
}

// Get entity by ID
Entity* ECS::GetEntity(EntityID id) {
    if (HasEntity(id)) {
        return &componentStorage<Entity>()[id];
    }
    return nullptr;
}

// Destroy an entity and clean up associated components
void ECS::DestroyEntity(EntityID id) {
    if (HasEntity(id)) {
        componentStorage<Entity>().erase(id);
        componentStorage<PhysicsComponent>().erase(id);
        componentStorage<TransformComponent>().erase(id);
        componentStorage<CameraComponent>().erase(id);
    }
}

// Clear all entities and components
void ECS::Clear() {
    componentStorage<Entity>().clear();
    componentStorage<PhysicsComponent>().clear();
    componentStorage<TransformComponent>().clear();
    componentStorage<CameraComponent>().clear();
}

// Add components
void ECS::AddPhysics(EntityID id, const PhysicsComponent& p) {
    if (!HasPhysics(id)) {
        componentStorage<PhysicsComponent>()[id] = p;
    }
}

PhysicsComponent* ECS::GetPhysics(EntityID id) {
    if (HasPhysics(id)) {
        return &componentStorage<PhysicsComponent>()[id];
    }
    return nullptr;
}

bool ECS::HasPhysics(EntityID id) {
    return componentStorage<PhysicsComponent>().find(id) != componentStorage<PhysicsComponent>().end();
}

void ECS::AddTransform(EntityID id, const TransformComponent& t) {
    if (!HasTransform(id)) {
        componentStorage<TransformComponent>()[id] = t;
    }
}

TransformComponent* ECS::GetTransform(EntityID id) {
    if (HasTransform(id)) {
        return &componentStorage<TransformComponent>()[id];
    }
    return nullptr;
}

bool ECS::HasTransform(EntityID id) {
    return componentStorage<TransformComponent>().find(id) != componentStorage<TransformComponent>().end();
}

void ECS::AddCamera(EntityID id, const CameraComponent& c) {
    if (!HasCamera(id)) {
        componentStorage<CameraComponent>()[id] = c;
    }
}

CameraComponent* ECS::GetCamera(EntityID id) {
    if (HasCamera(id)) {
        return &componentStorage<CameraComponent>()[id];
    }
    return nullptr;
}

bool ECS::HasCamera(EntityID id) {
    return componentStorage<CameraComponent>().find(id) != componentStorage<CameraComponent>().end();
}

// Get all entities
const std::unordered_map<EntityID, Entity>& ECS::GetAllEntities() {
    return componentStorage<Entity>();
}

// Get component by type
template <typename T>
T* ECS::GetComponent(EntityID id) {
    auto& components = componentStorage<T>();
    auto it = components.find(id);
    return (it != components.end()) ? &it->second : nullptr;
}

// Get the tag of an entity
std::string ECS::GetTag(EntityID id) {
    if (HasEntity(id)) {
        return componentStorage<Entity>()[id].tag;
    }
    return "";
}

// Set the tag of an entity
void ECS::SetTag(EntityID id, const std::string& tag) {
    if (HasEntity(id)) {
        componentStorage<Entity>()[id].tag = tag;
    }
}
