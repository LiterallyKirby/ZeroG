#include "RenderSystem.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

RenderSystem::RenderSystem() {
    shaderProgram = compileShader("shaders/vertex.glsl", "shaders/fragment.glsl");
    if (!shaderProgram) {
        std::cerr << "Failed to create shader program\n";
        return;
    }
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    viewLoc  = glGetUniformLocation(shaderProgram, "view");
    projLoc  = glGetUniformLocation(shaderProgram, "projection");
}

RenderSystem::~RenderSystem() {
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

void RenderSystem::RenderEntity(const Entity& e, const TransformComponent& t, const CameraComponent* cam) {
    if (!shaderProgram || !e.mesh) return;
    const auto& mesh = e.mesh;
    if (mesh->VAO == 0 || mesh->indexCount == 0) return;

    glUseProgram(shaderProgram);

    glm::mat4 model(1.0f);
    model = glm::translate(model, t.position);
    model = glm::rotate(model, glm::radians(t.rotation.x), glm::vec3(1,0,0));
    model = glm::rotate(model, glm::radians(t.rotation.y), glm::vec3(0,1,0));
    model = glm::rotate(model, glm::radians(t.rotation.z), glm::vec3(0,0,1));
    model = glm::scale(model, t.scale);

    glm::mat4 view(1.0f), proj(1.0f);
    if (cam) { view = cam->GetView(); proj = cam->GetProj(); }

    if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    if (viewLoc  >= 0) glUniformMatrix4fv(viewLoc,  1, GL_FALSE, glm::value_ptr(view));
    if (projLoc  >= 0) glUniformMatrix4fv(projLoc,  1, GL_FALSE, glm::value_ptr(proj));

    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

unsigned int RenderSystem::compileShader(const char* vertexPath, const char* fragmentPath) {
    auto compileSrc = [](GLenum type, const char* src)->GLuint {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        int success; glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512]; glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "Shader compile error: " << log << "\n";
            glDeleteShader(shader); return 0;
        }
        return shader;
    };

    std::string v = readFile(vertexPath), f = readFile(fragmentPath);
    if (v.empty() || f.empty()) { std::cerr << "Shader file empty/not found\n"; return 0; }

    GLuint vs = compileSrc(GL_VERTEX_SHADER, v.c_str());
    GLuint fs = compileSrc(GL_FRAGMENT_SHADER, f.c_str());
    if (!vs || !fs) return 0;

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs); glLinkProgram(prog);

    int success; glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) { char log[512]; glGetProgramInfoLog(prog, 512, nullptr, log);
        std::cerr << "Program link error: " << log << "\n"; glDeleteProgram(prog); prog = 0; }

    glDeleteShader(vs); glDeleteShader(fs);
    return prog;
}

std::string RenderSystem::readFile(const char* filepath) {
    std::ifstream f(filepath); if (!f) return {};
    std::stringstream ss; ss << f.rdbuf(); return ss.str();
}
