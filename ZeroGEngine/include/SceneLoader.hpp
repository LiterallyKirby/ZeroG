
#pragma once
#include "EntityBuilder.hpp"
#include "PhysicsComponent.hpp"
#include "nlohmann/json.hpp"
#include <string>

class SceneLoader {
public:
    static void LoadScene(const std::string& filename);
};
