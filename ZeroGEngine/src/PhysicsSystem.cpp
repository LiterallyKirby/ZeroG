// src/PhysicsSystem.cpp
#include "PhysicsSystem.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>
#include <glm/gtx/euler_angles.hpp>
#include <iostream>

using namespace physx;

// ---------- Simple synchronous dispatcher (no PhysXExtensions) ----------
class SimpleCpuDispatcher : public PxCpuDispatcher {
public:
  SimpleCpuDispatcher(PxU32 workerCount = 1) : mWorkerCount(workerCount) {}
  virtual void submitTask(PxBaseTask &task) override {
    task.run();
    task.release();
  }
  virtual PxU32 getWorkerCount() const override { return mWorkerCount; }

private:
  PxU32 mWorkerCount;
};

// ---------- Improved filter shader ----------
static PxFilterFlags SimpleFilterShader(PxFilterObjectAttributes attributes0,
                                        PxFilterData filterData0,
                                        PxFilterObjectAttributes attributes1,
                                        PxFilterData filterData1,
                                        PxPairFlags &pairFlags,
                                        const void * /*constantBlock*/,
                                        PxU32 /*constantBlockSize*/) {

  // Enable all contact reports and collision resolution
  pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_CONTACT_POINTS;

  // Handle trigger volumes
  if ((attributes0 & PxFilterObjectFlag::eTRIGGER) ||
      (attributes1 & PxFilterObjectFlag::eTRIGGER)) {
    pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
    return PxFilterFlags();
  }

  // Enable CCD if either object is flagged for it
  if ((filterData0.word3 & 1) || (filterData1.word3 & 1)) {

    pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
  }

  return PxFilterFlags();
}

// ---------- Minimal Error Callback ----------
class SimpleErrorCallback : public PxErrorCallback {
public:
  virtual void reportError(PxErrorCode::Enum code, const char *message,
                           const char *file, int line) override {
    std::cerr << "[PhysX] Error (" << static_cast<int>(code) << ") "
              << (message ? message : "(null)") << " at "
              << (file ? file : "(unknown)") << ":" << line << "\n";
  }
};

// ---------- Utility conversions ----------
static inline PxVec3 glmToPxVec3(const glm::vec3 &v) {
  return PxVec3(v.x, v.y, v.z);
}
static inline glm::vec3 pxToGlmVec3(const PxVec3 &v) {
  return glm::vec3(v.x, v.y, v.z);
}

// ---------- PhysicsSystem implementation ----------
PhysicsSystem::PhysicsSystem() {
  // Reserve some capacity to avoid frequent reallocations
  dynamicEntities.reserve(100);
  entityToActor.reserve(100);
}

void PhysicsSystem::Init() {
  static PxDefaultAllocator gAllocator;
  static SimpleErrorCallback gErrorCallback;

  foundation =
      PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
  if (!foundation) {
    std::cerr << "PxCreateFoundation failed\n";
    return;
  }

  PxTolerancesScale toleranceScale;
  toleranceScale.length = 1.0f; // Typical object size is 1 unit
  toleranceScale.speed = 10.0f; // Typical speed is 10 units/second
  physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, toleranceScale,
                            true, nullptr);
  if (!physics) {
    std::cerr << "PxCreatePhysics failed\n";
    return;
  }

  dispatcher = new SimpleCpuDispatcher(1);

  PxSceneDesc sceneDesc(physics->getTolerancesScale());
  sceneDesc.gravity = PxVec3(0.f, -9.81f, 0.f);
  sceneDesc.cpuDispatcher = dispatcher;
  sceneDesc.filterShader = SimpleFilterShader;

  // Critical: Increase solver iterations to prevent interpenetration
  sceneDesc.solverType = PxSolverType::ePGS;
  // Note: Some PhysX versions use different property names
  // If your version doesn't have these properties, remove these lines:
  // sceneDesc.solverPositionIterations = 8;
  // sceneDesc.solverVelocityIterations = 2;

  // Enable CCD (Continuous Collision Detection) for fast-moving objects
  sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;

  scene = physics->createScene(sceneDesc);
  if (!scene) {
    std::cerr << "createScene failed\n";
    return;
  }

  material = physics->createMaterial(0.5f, 0.5f, 0.6f);
  if (!material) {
    std::cerr << "createMaterial failed\n";
    return;
  }
}

void PhysicsSystem::CreatePhysXActor(Entity &entity,
                                     PhysicsComponent &physicsComp) {
  if (!physics || !scene)
    return;

  // Check if actor already exists
  if (entityToActor.find(entity.id) != entityToActor.end()) {
    std::cerr << "Actor already exists for entity " << entity.id << "\n";
    return;
  }

  auto *t = ECS::GetTransform(entity.id);
  if (!t)
    return;

  PhysicsOptions opts;
  if (physicsComp.options) {
    opts = *physicsComp.options;
  } else {
    opts.shapeType = PhysicsOptions::ShapeType::BOX;
    opts.dimensions =
        (t->scale == glm::vec3(0.0f)) ? glm::vec3(1.0f) : t->scale;
    opts.mass = physicsComp.mass;
    opts.isStatic = physicsComp.isStatic;
  }

  // Safety: ensure dimensions are valid
  if (opts.dimensions.x <= 0.0f)
    opts.dimensions.x = 1.0f;
  if (opts.dimensions.y <= 0.0f)
    opts.dimensions.y = 1.0f;
  if (opts.dimensions.z <= 0.0f)
    opts.dimensions.z = 1.0f;

  PxMaterial *mat = physics->createMaterial(
      opts.staticFriction, opts.dynamicFriction, opts.restitution);
  if (!mat)
    return;

  // Ensure material has proper contact generation
  mat->setFrictionCombineMode(PxCombineMode::eAVERAGE);
  mat->setRestitutionCombineMode(PxCombineMode::eAVERAGE);

  PxTransform pose(glmToPxVec3(t->position));
  PxRigidActor *actor = nullptr;

  std::cout << "Creating actor at position: (" << t->position.x << ", "
            << t->position.y << ", " << t->position.z << ") - "
            << (opts.isStatic ? "STATIC" : "DYNAMIC") << std::endl;
  std::cout << "  Mass: " << opts.mass << ", isStatic flag: " << opts.isStatic
            << std::endl;

  if (!opts.isStatic || opts.isKinematic) {
    // Dynamic or kinematic actor
    PxRigidDynamic *body = physics->createRigidDynamic(pose);
    if (!body) {
      std::cerr << "ERROR: Failed to create PxRigidDynamic" << std::endl;
      mat->release();
      return;
    }

    // Create shape based on type
    PxShape *shape = nullptr;
    switch (opts.shapeType) {
    case PhysicsOptions::ShapeType::BOX: {
      PxVec3 halfExtents(opts.dimensions.x * 0.5f, opts.dimensions.y * 0.5f,
                         opts.dimensions.z * 0.5f);
      std::cout << "  Box half-extents: (" << halfExtents.x << ", "
                << halfExtents.y << ", " << halfExtents.z << ")" << std::endl;
      shape = physics->createShape(PxBoxGeometry(halfExtents), *mat);
      break;
    }
    case PhysicsOptions::ShapeType::SPHERE:
      std::cout << "  Sphere radius: " << opts.dimensions.x << std::endl;
      shape = physics->createShape(PxSphereGeometry(opts.dimensions.x), *mat);
      break;
    case PhysicsOptions::ShapeType::CAPSULE:
      std::cout << "  Capsule radius: " << opts.dimensions.x
                << ", half-height: " << opts.dimensions.y * 0.5f << std::endl;
      shape = physics->createShape(
          PxCapsuleGeometry(opts.dimensions.x, opts.dimensions.y * 0.5f), *mat);
      break;
    }

    if (!shape) {
      std::cerr << "ERROR: Failed to create shape for dynamic body"
                << std::endl;
      body->release();
      mat->release();
      return;
    }

    body->attachShape(*shape);

    // Enable contact reporting for this shape
    shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
    shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

    shape->release();

    body->setMass(opts.mass > 0.0f ? opts.mass : 1.0f);

    // Improve collision stability - using only widely supported methods
    body->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD,
                           true); // Continuous collision detection

    // Set reasonable damping to prevent excessive bouncing
    body->setLinearDamping(0.1f);
    body->setAngularDamping(0.1f);

    if (opts.isKinematic) {
      body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    }

    actor = body;

    // Track dynamic/kinematic actors for updates
    dynamicEntities.push_back(entity.id);
    std::cout << "  Added to dynamic entities list" << std::endl;
  } else {
    // Static actor
    PxRigidStatic *body = physics->createRigidStatic(pose);
    if (!body) {
      mat->release();
      return;
    }

    // Create shape
    PxShape *shape = nullptr;
    switch (opts.shapeType) {
    case PhysicsOptions::ShapeType::BOX: {
      PxVec3 halfExtents(opts.dimensions.x * 0.5f, opts.dimensions.y * 0.5f,
                         opts.dimensions.z * 0.5f);
      std::cout << "  Static box half-extents: (" << halfExtents.x << ", "
                << halfExtents.y << ", " << halfExtents.z << ")" << std::endl;
      shape = physics->createShape(PxBoxGeometry(halfExtents), *mat);
      break;
    }
    case PhysicsOptions::ShapeType::SPHERE:
      std::cout << "  Static sphere radius: " << opts.dimensions.x << std::endl;
      shape = physics->createShape(PxSphereGeometry(opts.dimensions.x), *mat);
      break;
    case PhysicsOptions::ShapeType::CAPSULE:
      std::cout << "  Static capsule radius: " << opts.dimensions.x
                << ", half-height: " << opts.dimensions.y * 0.5f << std::endl;
      shape = physics->createShape(
          PxCapsuleGeometry(opts.dimensions.x, opts.dimensions.y * 0.5f), *mat);
      break;
    }

    if (!shape) {
      std::cerr << "ERROR: Failed to create shape for static body" << std::endl;
      body->release();
      mat->release();
      return;
    }

    body->attachShape(*shape);

    // Enable contact reporting for static shapes too
    shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
    shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

    shape->release();

    actor = body;
    std::cout << "  Created static actor successfully" << std::endl;
  }

  // Add to scene and track
  scene->addActor(*actor); // addActor returns void, not bool
  std::cout << "  Actor added to scene successfully" << std::endl;

  entityToActor[entity.id] = actor;
  physicsComp.pxActor = actor;

  std::cout << "  Actor registered in entityToActor map with ID: " << entity.id
            << std::endl;
  std::cout << "  Map size is now: " << entityToActor.size() << std::endl;

  mat->release();
}

void PhysicsSystem::RemoveActor(EntityID entityId) {
  // Mark for removal to avoid modifying containers during iteration
  pendingRemovals.insert(entityId);
}

void PhysicsSystem::ProcessPendingRemovals() {
  for (EntityID entityId : pendingRemovals) {
    auto it = entityToActor.find(entityId);
    if (it != entityToActor.end()) {
      CleanupActor(entityId, it->second);
      entityToActor.erase(it);
    }

    // Remove from dynamic entities list
    auto dynIt =
        std::find(dynamicEntities.begin(), dynamicEntities.end(), entityId);
    if (dynIt != dynamicEntities.end()) {
      // Efficient removal: swap with last element and pop
      if (dynIt != dynamicEntities.end() - 1) {
        *dynIt = dynamicEntities.back();
      }
      dynamicEntities.pop_back();
    }
  }
  pendingRemovals.clear();
}

void PhysicsSystem::CleanupActor(EntityID entityId, PxRigidActor *actor) {
  if (!actor)
    return;

  // Remove from scene
  if (scene && actor->getScene() == scene) {
    scene->removeActor(*actor);
  }

  // Release the actor
  actor->release();

  // Clear physics component reference
  auto *physicsComp = ECS::GetPhysics(entityId);
  if (physicsComp) {
    physicsComp->pxActor = nullptr;
  }
}

void PhysicsSystem::FixedUpdate(float timestep) {
  if (!scene)
    return;

  // Process any pending removals first
  ProcessPendingRemovals();

  // Run physics simulation
  scene->simulate(timestep);
  scene->fetchResults(true);

  // Check for any penetrations (debug)
  static int debugCounter = 0;
  if (debugCounter % 120 == 0) { // Every 2 seconds at 60Hz
    std::cout << "=== Physics Debug Info ===" << std::endl;
    for (const auto &pair : entityToActor) {
      auto *transform = ECS::GetTransform(pair.first);
      auto *entity = ECS::GetEntity(pair.first);
      if (transform && entity) {
        std::cout << entity->tag << " at (" << transform->position.x << ", "
                  << transform->position.y << ", " << transform->position.z
                  << ")" << std::endl;
      }
    }
    std::cout << "=========================" << std::endl;
  }
  debugCounter++;

  // Update only dynamic entities (much faster than checking all entities)
  for (auto it = dynamicEntities.begin(); it != dynamicEntities.end();) {
    EntityID entityId = *it;

    // Check if entity still exists
    auto *t = ECS::GetTransform(entityId);
    auto *physicsComp = ECS::GetPhysics(entityId);

    if (!t || !physicsComp) {
      // Entity was destroyed, remove it
      RemoveActor(entityId);
      // Note: actual removal happens in ProcessPendingRemovals()
      ++it;
      continue;
    }

    // Get the actor
    auto actorIt = entityToActor.find(entityId);
    if (actorIt == entityToActor.end()) {
      // Actor missing, remove from dynamic list
      it = dynamicEntities.erase(it);
      continue;
    }

    PxRigidActor *actor = actorIt->second;
    if (!actor) {
      // Invalid actor, clean up
      RemoveActor(entityId);
      ++it;
      continue;
    }

    // Verify actor is still in scene
    if (actor->getScene() != scene) {
      RemoveActor(entityId);
      ++it;
      continue;
    }

    // Cast to dynamic actor for position updates
    PxRigidDynamic *dynamicActor = actor->is<PxRigidDynamic>();
    if (dynamicActor) {
      // Update transform from physics
      PxTransform pxT = dynamicActor->getGlobalPose();
      glm::vec3 oldPos = t->position;
      t->position = pxToGlmVec3(pxT.p);

      // Debug output - only print if position changed significantly
      if (glm::distance(oldPos, t->position) > 0.1f) {
        auto *entity = ECS::GetEntity(entityId);
        std::string name = entity ? entity->tag : "Unknown";
        std::cout << "Entity " << name << " moved to (" << t->position.x << ", "
                  << t->position.y << ", " << t->position.z << ")" << std::endl;
      }

      // Handle kinematic actors
      if (dynamicActor->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC) {
        // For kinematic bodies, update the target from current transform
        PxTransform target(glmToPxVec3(t->position));
        dynamicActor->setKinematicTarget(target);
      }

      // Update rotation
      PxQuat q = pxT.q;
      glm::quat gq(q.w, q.x, q.y, q.z);
      glm::vec3 eulerRad = glm::eulerAngles(gq);
      t->rotation = glm::degrees(eulerRad);
    }

    ++it;
  }
}

void PhysicsSystem::Integrate(Entity &entity, PhysicsComponent &physicsComp,
                              float deltaTime) {
  if (physicsComp.isStatic)
    return;

  auto *t = ECS::GetTransform(entity.id);
  if (!t)
    return;

  physicsComp.acceleration = physicsComp.forces / physicsComp.mass;
  physicsComp.velocity += physicsComp.acceleration * deltaTime;
  t->position += physicsComp.velocity * deltaTime;
  physicsComp.ClearForces();
}

PhysicsSystem::~PhysicsSystem() {
  // Clean up all actors
  for (auto &pair : entityToActor) {
    if (pair.second) {
      CleanupActor(pair.first, pair.second);
    }
  }
  entityToActor.clear();
  dynamicEntities.clear();
  pendingRemovals.clear();

  if (scene)
    scene->release();
  if (dispatcher)
    delete dispatcher;
  if (physics)
    physics->release();
  if (foundation)
    foundation->release();
}
