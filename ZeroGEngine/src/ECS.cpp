#include "ECS.hpp"

namespace {
    std::unordered_map<EntityID, Entity> entities;
    EntityID nextEntityID = 1;
}

namespace ECS {


EntityID CreateEntity(std::shared_ptr<Mesh> mesh, const std::string& tag) {
    EntityID id = nextEntityID++;
    entities[id] = Entity{id, mesh, tag};
    return id;
}


bool HasEntity(EntityID id) {
    return entities.find(id) != entities.end();
}

Entity* GetEntity(EntityID id) {
    auto it = entities.find(id);
    return it != entities.end() ? &it->second : nullptr;
}

void AddTransform(EntityID id, const TransformComponent& t) {
    if (auto e = GetEntity(id)) {
        e->transform = t;
    }
}
std::string GetTag(EntityID id) {
    if (auto e = GetEntity(id)) {
        return e->tag;
    }
    return "";
}

void SetTag(EntityID id, const std::string& tag) {
    if (auto e = GetEntity(id)) {
        e->tag = tag;
    }
}
TransformComponent* GetTransform(EntityID id) {
    if (auto e = GetEntity(id)) {
        return &e->transform;
    }
    return nullptr;
}
void Clear() {
    entities.clear();
    nextEntityID = 1;
}
bool HasTransform(EntityID id) {
    if (auto e = GetEntity(id)) {
        return true;
    }
    return false;
}

void AddPhysics(EntityID id, const PhysicsComponent& p) {
    if (auto e = GetEntity(id)) {
        e->physics = std::make_shared<PhysicsComponent>(p);
    }
}

PhysicsComponent* GetPhysics(EntityID id) {
    if (auto e = GetEntity(id)) {
        return e->physics.get();
    }
    return nullptr;
}

bool HasPhysics(EntityID id) {
    return GetPhysics(id) != nullptr;
}

// ===== CAMERA =====
void AddCamera(EntityID id, const CameraComponent& c) {
    if (auto e = GetEntity(id)) {
        e->camera = std::make_shared<CameraComponent>(c);
        e->hasCamera = true;
    }
}

CameraComponent* GetCamera(EntityID id) {
    if (auto e = GetEntity(id)) {
        return e->camera.get();
    }
    return nullptr;
}

bool HasCamera(EntityID id) {
    return GetCamera(id) != nullptr;
}

// iterate
const std::unordered_map<EntityID, Entity>& GetAllEntities() {
    return entities;
}

// destroy
void DestroyEntity(EntityID id) {
    entities.erase(id);
}

} // namespace ECS
