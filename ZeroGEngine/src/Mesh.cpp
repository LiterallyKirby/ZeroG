#include "Mesh.hpp"
#include <iostream>
#include "cmath"
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


static void buildSphere(std::vector<float>& v, std::vector<unsigned int>& i, int latBands = 16, int longBands = 16) {
    v.clear();
    i.clear();

    for (int lat = 0; lat <= latBands; ++lat) {
        float theta = lat * M_PI / latBands;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= longBands; ++lon) {
            float phi = lon * 2 * M_PI / longBands;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            // position + color
            v.push_back(x * 0.5f); // scale to radius=0.5
            v.push_back(y * 0.5f);
            v.push_back(z * 0.5f);

            v.push_back((float)lat / latBands); // R
            v.push_back((float)lon / longBands); // G
            v.push_back(1.0f); // B
        }
    }

    for (int lat = 0; lat < latBands; ++lat) {
        for (int lon = 0; lon < longBands; ++lon) {
            int first = (lat * (longBands + 1)) + lon;
            int second = first + longBands + 1;

            i.push_back(first);
            i.push_back(second);
            i.push_back(first + 1);

            i.push_back(second);
            i.push_back(second + 1);
            i.push_back(first + 1);
        }
    }
}


#include "Mesh.hpp"

// helper builders (buildCube, buildPyramid, buildSphere)...

Mesh::Mesh(MeshType t) : type(t) {
    if (type == MeshType::Cube) buildCube(vertices, indices);
    else if (type == MeshType::Pyramid) buildPyramid(vertices, indices);
    else if (type == MeshType::Sphere) buildSphere(vertices, indices);

    indexCount = static_cast<unsigned int>(indices.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);

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

// move ctor / move assign: make sure they use the same members
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

