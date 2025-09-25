#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include "CameraComponent.hpp"
#include "ECS.hpp"
#include "EntityBuilder.hpp"
#include "MeshType.hpp"
#include "PhysicsSystem.hpp"
#include "Scene.hpp"
#include "tinyfiledialogs.h" // ← include file picker
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

// Global physics system reference for entity creation
PhysicsSystem *g_physicsSystem = nullptr;


void LoadSceneFromJSON(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open scene file: " << filename << "\n";
    return;
  }

  json sceneJson;
  file >> sceneJson;

  std::cout << "Loading scene from " << filename << "..." << std::endl;

  // Store entity IDs for physics setup
  std::vector<unsigned int> physicsEntities;

  // Load camera
  if (sceneJson.contains("camera")) {
    auto camJson = sceneJson["camera"];
    auto pos = camJson.value("position", std::vector<float>{0.0f, 0.0f, 5.0f});
    auto rot = camJson.value("rotation", std::vector<float>{0.0f, 0.0f, 0.0f});
    float fov = camJson.value("fov", 60.0f);

    std::cout << "Creating camera: pos(" << pos[0] << ", " << pos[1] << ", "
              << pos[2] << ") "
              << "rot(" << rot[0] << ", " << rot[1] << ", "
              << (rot.size() > 2 ? rot[2] : 0.0f) << ") "
              << "fov(" << fov << ")" << std::endl;

    glm::vec3 position{pos[0], pos[1], pos.size() > 2 ? pos[2] : 0.f};
    glm::vec3 rotation{rot[0], rot[1], rot.size() > 2 ? rot[2] : 0.f};

    EntityBuilder builder;
    builder.WithTag("MainCamera");
    builder.WithCamera(CameraComponent(fov, position, rotation));
    auto cameraId = builder.Build();

    std::cout << "Camera created with ID: " << cameraId << std::endl;

    // Verify camera
    if (ECS::HasCamera(cameraId)) {
      auto *cam = ECS::GetCamera(cameraId);
      std::cout << "Camera verified - Front: (" << cam->front.x << ", "
                << cam->front.y << ", " << cam->front.z << ")" << std::endl;
    }
  }

  // Load entities
  if (sceneJson.contains("entities")) {
    int entityCount = 0;
    for (const auto &entityJson : sceneJson["entities"]) {
      EntityBuilder builder;

      // Mesh
      std::string meshType = entityJson.value("mesh", "Cube");
      if (meshType == "Cube") {
        builder.WithMesh(MeshType::Cube);
      } else if (meshType == "Pyramid") {
        builder.WithMesh(MeshType::Pyramid);
      }

      // Position
      auto pos = entityJson.value("position", std::vector<float>{0.0f, 0.0f, 0.0f});
      builder.WithPosition(pos[0], pos[1], pos.size() > 2 ? pos[2] : 0.f);

      // Scale
      auto scale = entityJson.value("scale", std::vector<float>{1.0f, 1.0f, 1.0f});
      builder.WithScale(scale[0], scale[1], scale.size() > 2 ? scale[2] : 1.f);

      // Tag
      std::string tag = entityJson.value("tag", "");
      builder.WithTag(tag);

      // Physics - FIXED LOGIC
      if (entityJson.contains("physics")) {
        auto physJson = entityJson["physics"];
        PhysicsOptions opts;

        // Mass determines if static or dynamic
        float mass = physJson.value("mass", 1.0f);
        opts.mass = mass;
        
        // CRITICAL FIX: If mass is 0, force static regardless of JSON
        if (mass <= 0.0f) {
          opts.isStatic = true;
          opts.isKinematic = false;
          std::cout << "Creating STATIC body for '" << tag << "' (mass = " << mass << ")" << std::endl;
        } else {
          opts.isStatic = physJson.value("isStatic", false); // Allow JSON override for non-zero mass
          opts.isKinematic = physJson.value("isKinematic", false);
          std::cout << "Creating DYNAMIC body for '" << tag << "' (mass = " << mass 
                    << ", isStatic=" << opts.isStatic << ", isKinematic=" << opts.isKinematic << ")" << std::endl;
        }

        // Friction/restitution
        opts.staticFriction = physJson.value("staticFriction", 0.5f);
        opts.dynamicFriction = physJson.value("dynamicFriction", 0.5f);
        opts.restitution = physJson.value("restitution", 0.1f);

        // Shape type
        std::string shapeStr = physJson.value("shape", "box");
        if (shapeStr == "box") {
          opts.shapeType = PhysicsOptions::ShapeType::BOX;
        } else if (shapeStr == "sphere") {
          opts.shapeType = PhysicsOptions::ShapeType::SPHERE;
        } else if (shapeStr == "capsule") {
          opts.shapeType = PhysicsOptions::ShapeType::CAPSULE;
        }

        // Dimensions - CRITICAL FIX
        if (physJson.contains("dimensions")) {
          auto dims = physJson["dimensions"].get<std::vector<float>>();
          if (opts.shapeType == PhysicsOptions::ShapeType::BOX) {
            if (dims.size() >= 3) {
              opts.dimensions = glm::vec3(dims[0], dims[1], dims[2]);
            } else {
              opts.dimensions = glm::vec3(scale[0], scale[1], scale[2]);
            }
          } else if (opts.shapeType == PhysicsOptions::ShapeType::SPHERE) {
            if (!dims.empty()) {
              opts.dimensions.x = dims[0]; // radius
            } else {
              opts.dimensions.x = std::max({scale[0], scale[1], scale[2]}) * 0.5f;
            }
          } else if (opts.shapeType == PhysicsOptions::ShapeType::CAPSULE) {
            if (dims.size() >= 2) {
              opts.dimensions = glm::vec3(dims[0], dims[1], 0.0f); // radius, height
            } else {
              opts.dimensions = glm::vec3(scale[0] * 0.5f, scale[1], 0.0f);
            }
          }
        } else {
          // Use scale as dimensions for all shapes
          if (opts.shapeType == PhysicsOptions::ShapeType::BOX) {
            opts.dimensions = glm::vec3(scale[0], scale[1], scale[2]);
          } else if (opts.shapeType == PhysicsOptions::ShapeType::SPHERE) {
            opts.dimensions.x = std::max({scale[0], scale[1], scale[2]}) * 0.5f;
          } else if (opts.shapeType == PhysicsOptions::ShapeType::CAPSULE) {
            opts.dimensions = glm::vec3(scale[0] * 0.5f, scale[1], 0.0f);
          }
        }

        std::cout << "  Physics shape: " << shapeStr 
                  << ", dimensions: (" << opts.dimensions.x 
                  << ", " << opts.dimensions.y 
                  << ", " << opts.dimensions.z << ")" << std::endl;

        builder.WithPhysicsOptions(opts);
      }

      auto entityId = builder.Build();
      entityCount++;
      std::cout << "Created entity '" << tag << "' with ID: " << entityId << std::endl;

      // If entity has physics, add to our list for PhysX actor creation
      if (entityJson.contains("physics")) {
        physicsEntities.push_back(entityId);
      }
    }
    std::cout << "Created " << entityCount << " entities total." << std::endl;
  }

  // Create PhysX actors for all physics entities
  if (g_physicsSystem) {
    std::cout << "\nCreating PhysX actors..." << std::endl;
    for (unsigned int entityId : physicsEntities) {
      auto *physicsComp = ECS::GetPhysics(entityId);
      if (physicsComp) {
        Entity tempEntity;
        tempEntity.id = entityId;

        g_physicsSystem->CreatePhysXActor(tempEntity, *physicsComp);

        auto *transform = ECS::GetTransform(entityId);
        auto *entity = ECS::GetEntity(entityId);
        std::string entityTag = entity ? entity->tag : "Unknown";
        
        if (transform) {
          std::cout << "  ✓ Created PhysX actor for '" << entityTag << "' (ID: " << entityId
                    << ") at position (" << transform->position.x << ", "
                    << transform->position.y << ", " << transform->position.z
                    << ") - " << (physicsComp->isStatic ? "STATIC" : "DYNAMIC") << std::endl;
        }
        
        // Verify actor was created
        if (g_physicsSystem->HasActor(entityId)) {
          std::cout << "    ✓ Actor successfully registered" << std::endl;
        } else {
          std::cout << "    ✗ ERROR: Actor creation failed!" << std::endl;
        }
      }
    }
  }

  std::cout << "\nScene loading complete!" << std::endl;
}

int main() {
  if (!glfwInit()) {
    std::cerr << "GLFW init failed\n";
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(800, 600, "ZeroG ECS", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to init GLAD\n";
    return -1;
  }

  glEnable(GL_DEPTH_TEST);

  // Initialize physics system first
  PhysicsSystem physicsSystem;
  physicsSystem.Init();
  g_physicsSystem = &physicsSystem; // Set global reference for scene loading

  std::cout << "Physics system initialized." << std::endl;

  // ** Popup file picker **
  const char *file = tinyfd_openFileDialog(
      "Select Scene JSON", "", 1, (const char *[]){"*.json"}, "JSON Files", 0);

  if (!file) {
    std::cout << "No file selected, exiting...\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
  }

  LoadSceneFromJSON(file);

  Scene scene;

  std::cout << "Starting render loop... Press ESC to exit" << std::endl;

  const float fixedDeltaTime = 1.0f / 60.0f; // fixed 60 Hz physics
  float accumulator = 0.0f;

  double lastTime = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    double currentTime = glfwGetTime();
    float frameTime = static_cast<float>(currentTime - lastTime);
    lastTime = currentTime;

    // Cap frame time to prevent spiral of death
    frameTime = std::min(frameTime, 0.25f);

    accumulator += frameTime;

    // Fixed physics step using FixedUpdate
    while (accumulator >= fixedDeltaTime) {
      physicsSystem.FixedUpdate(fixedDeltaTime); // PhysX simulation step
      accumulator -= fixedDeltaTime;
    }

    // Normal rendering
    glClearColor(0.12f, 0.12f, 0.12f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.Render();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Clean up

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
