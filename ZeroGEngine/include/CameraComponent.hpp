#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CameraComponent {
    glm::vec3 position{1.5f, 1.5f, 1.5f};
    glm::vec3 front{-1.0f, -1.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    float fov{45.f};
    float aspect{800.f/600.f};
    float nearPlane{0.1f};
    float farPlane{100.f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f}; // pitch, yaw, roll in degrees
    
    CameraComponent() { 
        updateFront(); // Make sure default constructor also updates front
    }
    
    CameraComponent(float fovVal,
                    glm::vec3 pos,
                    glm::vec3 rotVec,
                    float aspectVal = 800.f/600.f,
                    float nearVal = 0.1f,
                    float farVal = 100.f)
        : position(pos), rotation(rotVec), fov(fovVal), aspect(aspectVal), nearPlane(nearVal), farPlane(farVal) {
        updateFront();
    }
    
    void updateFront() {
        float pitch = glm::radians(rotation.x);
        float yaw = glm::radians(rotation.y);
        
        front.x = cos(pitch) * sin(yaw);
        front.y = sin(pitch);
        front.z = -cos(pitch) * cos(yaw);
        front = glm::normalize(front);
    }
    
    // Add method to update rotation and front vector together
    void setRotation(const glm::vec3& newRotation) {
        rotation = newRotation;
        updateFront();
    }
    
    // Add method to update position
    void setPosition(const glm::vec3& newPosition) {
        position = newPosition;
    }
    
    glm::mat4 GetView() const { 
        return glm::lookAt(position, position + front, up); 
    }
    
    glm::mat4 GetProj() const { 
        return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane); 
    }
};
