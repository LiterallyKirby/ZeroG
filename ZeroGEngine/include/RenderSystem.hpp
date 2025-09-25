#pragma once
#include <glad/glad.h>// GLEW header (if you're using it alongside glad, otherwise skip it)


// Now, you can include other OpenGL-related headers or any other necessary headers

#include "TransformComponent.hpp"
#include "CameraComponent.hpp"
#include <string>

// Forward declarations to reduce unnecessary includes
class Entity;
class CameraComponent;

struct RenderSystem {
    unsigned int shaderProgram{0};        // The shader program ID
    int modelLoc, viewLoc, projLoc;  // Locations for transformation matrices

    // Light-related uniform locations
    GLuint lightPosLoc;
    GLuint lightDirLoc;
    GLuint lightColorLoc;
    GLuint ambientColorLoc;

    // Constructor and destructor for initializing and cleaning up resources
    RenderSystem();
    ~RenderSystem();

    // Render an entity with its transform and the current camera
    void RenderEntity(const Entity& e, const TransformComponent& t, const CameraComponent* cam);

private:
    // Private helper functions
    unsigned int compileShader(const char* vertexPath, const char* fragmentPath); // Compiles shaders from paths
    std::string readFile(const char* filepath);  // Reads file content for shader source
};
