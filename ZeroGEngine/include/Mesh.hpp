#pragma once
#include <vector>
#include <glad/glad.h>
#include "MeshType.hpp"

struct Mesh {
    MeshType type;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO{0}, VBO{0}, EBO{0};
    unsigned int indexCount{0};

    Mesh(MeshType t);
    Mesh(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept;
    Mesh& operator=(Mesh&&) noexcept;
    ~Mesh();
};
