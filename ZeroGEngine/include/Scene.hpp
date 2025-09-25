#pragma once
#include "RenderSystem.hpp"
#include "Entity.hpp"
#include "ECS.hpp"
#include "CameraComponent.hpp"
#include <vector>
#include "SceneLoader.hpp" // <-- add this

struct Scene {
    RenderSystem renderer;
    CameraComponent sceneCamera;

    Scene();
    void Render();
};
