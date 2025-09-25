#include "SceneLoader.hpp"
#include "ECS.hpp"
#include "CameraComponent.hpp"
#include <fstream>
#include <iostream>
using json = nlohmann::json;

void SceneLoader::LoadScene(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open scene file: " << filename << "\n";
        return;
    }
    
    json sceneJson;
    file >> sceneJson;
    
    // --- Camera Handling ---
    if (sceneJson.contains("camera")) {
        auto camJson = sceneJson["camera"];
        auto pos = camJson.value("position", std::vector<float>{0.0f, 0.0f, 5.0f});
        auto rot = camJson.value("rotation", std::vector<float>{0.0f, 0.0f, 0.0f});
        float fov = camJson.value("fov", 60.0f);
        
        // Debug output
        std::cout << "Loading camera: pos(" << pos[0] << ", " << pos[1] << ", " << pos[2] << ") "
                  << "rot(" << rot[0] << ", " << rot[1] << ", " << (rot.size() > 2 ? rot[2] : 0.0f) << ") "
                  << "fov(" << fov << ")\n";
        
        glm::vec3 position{pos[0], pos[1], pos.size() > 2 ? pos[2] : 0.f};
        glm::vec3 rotation{rot[0], rot[1], rot.size() > 2 ? rot[2] : 0.f};
        
        EntityBuilder builder;
        builder.WithTag("MainCamera");
        // Don't set transform position/rotation separately if camera handles its own
        builder.WithCamera(CameraComponent(fov, position, rotation));
        auto cameraEntity = builder.Build();
        
        // Verify the camera was created correctly
        if (ECS::HasCamera(cameraEntity)) {
            auto* cam = ECS::GetCamera(cameraEntity);
            std::cout << "Camera created successfully: pos(" << cam->position.x << ", " 
                      << cam->position.y << ", " << cam->position.z << ") "
                      << "front(" << cam->front.x << ", " << cam->front.y << ", " << cam->front.z << ")\n";
        } else {
            std::cerr << "Failed to create camera component!\n";
        }
    } else {
        std::cerr << "No camera found in scene file!\n";
    }
    
    // --- Entity Handling ---
    if (sceneJson.contains("entities")) {
        for (const auto& entityJson : sceneJson["entities"]) {
            EntityBuilder builder;
            
            // Mesh
            std::string meshType = entityJson.value("mesh", "Cube");
            if (meshType == "Cube") builder.WithMesh(MeshType::Cube);
            else if (meshType == "Pyramid") builder.WithMesh(MeshType::Pyramid);
            
            // Position
            auto pos = entityJson.value("position", std::vector<float>{0.0f, 0.0f, 0.0f});
            builder.WithPosition(pos[0], pos[1], pos.size() > 2 ? pos[2] : 0.f);
            
            // Scale
            auto scale = entityJson.value("scale", std::vector<float>{1.0f, 1.0f, 1.0f});
            builder.WithScale(scale[0], scale[1], scale.size() > 2 ? scale[2] : 1.f);
            
            // Tag
            builder.WithTag(entityJson.value("tag", ""));
            
            // Physics
            if (entityJson.contains("physics")) {
                auto physJson = entityJson["physics"];
                float mass = physJson.value("mass", 1.0f);
                builder.WithPhysics(PhysicsComponent(mass));
            }
            
            builder.Build();
        }
    }
}
