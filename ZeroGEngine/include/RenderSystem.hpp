#pragma once
#include "Entity.hpp"
#include "CameraComponent.hpp"
#include <glad/glad.h>
#include "TransformComponent.hpp"
struct RenderSystem {
    unsigned int shaderProgram{0};
    int modelLoc{-1}, viewLoc{-1}, projLoc{-1};

    RenderSystem();
    ~RenderSystem();

    void RenderEntity(const Entity& e, const TransformComponent& t, const CameraComponent* cam);
private:
    unsigned int compileShader(const char* vertexPath, const char* fragmentPath);
    std::string readFile(const char* filepath);
};
