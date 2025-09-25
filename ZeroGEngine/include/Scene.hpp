#pragma once
#include "RenderSystem.hpp"


#include "CameraComponent.hpp"
#include <vector>


struct Scene {
    RenderSystem renderer;
    CameraComponent sceneCamera;

    Scene();
    void Render();
};
