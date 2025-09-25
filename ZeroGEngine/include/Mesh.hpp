#pragma once
#include "MeshType.hpp"
#include <vector>
#include <glad/glad.h>

struct Mesh {
    MeshType type;
    std::vector<float> vertices;            // pos(3) + color(3)
    std::vector<unsigned int> indices;

    unsigned int VAO{0}, VBO{0}, EBO{0};
    unsigned int indexCount{0};

    Mesh(MeshType t);
    ~Mesh();

    // non-copyable, moveable
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
};
