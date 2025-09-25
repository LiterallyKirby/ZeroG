#ifndef TEXTURECOMPONENT_HPP
#define TEXTURECOMPONENT_HPP

// Include GLAD before any OpenGL-related headers
#include <glad/glad.h> // GLAD will load OpenGL functions

// stb_image for texture loading
#include "stb_image.h"

#include <string>

struct TextureComponent {
    GLuint textureID; // OpenGL texture ID
    std::string texturePath; // Path to the texture file

    // Constructor for creating a texture component
    TextureComponent(const std::string& path)
        : texturePath(path), textureID(0) {}

    // Load texture from a file
    void LoadTexture();
};

#endif // TEXTURECOMPONENT_HPP
