#include "Mesh.hpp"
#include <iostream>

static void buildCube(std::vector<float>& v, std::vector<unsigned int>& i) {
    v = {
        // positions         // colors
        -0.5f, -0.5f, -0.5f,  1,0,0, //0
         0.5f, -0.5f, -0.5f,  0,1,0, //1
         0.5f,  0.5f, -0.5f,  0,0,1, //2
        -0.5f,  0.5f, -0.5f,  1,1,0, //3
        -0.5f, -0.5f,  0.5f,  1,0,1, //4
         0.5f, -0.5f,  0.5f,  0,1,1, //5
         0.5f,  0.5f,  0.5f,  0.5,0.5,0.5, //6
        -0.5f,  0.5f,  0.5f,  0.9,0.3,0.2  //7
    };
    i = {
        4,5,6, 4,6,7,
        0,1,2, 0,2,3,
        0,4,7, 0,7,3,
        1,5,6, 1,6,2,
        3,2,6, 3,6,7,
        0,1,5, 0,5,4
    };
}

static void buildPyramid(std::vector<float>& v, std::vector<unsigned int>& i) {
    v = {
        // base 0..3, apex 4
        -0.5f, 0.0f, -0.5f,  1,0,0, //0
         0.5f, 0.0f, -0.5f,  0,1,0, //1
         0.5f, 0.0f,  0.5f,  0,0,1, //2
        -0.5f, 0.0f,  0.5f,  1,1,0, //3
         0.0f, 0.8f,  0.0f,  1,0.5,0.2 //4 apex
    };
    i = {
        0,1,2, 0,2,3, // base
        0,1,4,
        1,2,4,
        2,3,4,
        3,0,4
    };
}

Mesh::Mesh(MeshType t) : type(t) {
    if (type == MeshType::Cube) buildCube(vertices, indices);
    else buildPyramid(vertices, indices);

    indexCount = static_cast<unsigned int>(indices.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // pos(3) color(3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

Mesh::Mesh(Mesh&& o) noexcept {
    type = o.type;
    vertices = std::move(o.vertices);
    indices = std::move(o.indices);
    VAO = o.VAO; VBO = o.VBO; EBO = o.EBO; indexCount = o.indexCount;
    o.VAO = o.VBO = o.EBO = 0; o.indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept {
    if (this != &o) {
        if (EBO) glDeleteBuffers(1, &EBO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);

        type = o.type;
        vertices = std::move(o.vertices);
        indices = std::move(o.indices);
        VAO = o.VAO; VBO = o.VBO; EBO = o.EBO; indexCount = o.indexCount;
        o.VAO = o.VBO = o.EBO = 0; o.indexCount = 0;
    }
    return *this;
}
