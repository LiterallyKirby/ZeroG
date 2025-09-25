#pragma once
#include <glm/glm.hpp>

struct PhysicsOptions {
  enum class ShapeType { BOX, SPHERE, CAPSULE };

  ShapeType shapeType = ShapeType::BOX;
  // For BOX: dimensions = full size (x,y,z)
  // For SPHERE: dimensions.x = radius
  // For CAPSULE: dimensions.x = radius, dimensions.y = height (distance between
  // sphere centers)
  glm::vec3 dimensions{1.0f, 1.0f, 1.0f};

  bool isKinematic = false; // default false
  float mass = 1.0f;
  float staticFriction = 0.5f;
  float dynamicFriction = 0.5f;
  float restitution = 0.6f;
  bool isStatic = false;
};
