#pragma once

#include "particle.h"
#include "../core/shader.h"
#include "../core/core.h"

class ParticleSystem {
public:
    void init();
    void update(float dt);
    void render(Shader* particleShader, const glm::mat4& viewProj, const glm::vec3& camRight, const glm::vec3& camUp);
    void emit(glm::vec3 origin, int count = 1);

private:
    std::vector<Particle> particles;
    GLuint VAO, VBO;
    void setupMesh();
};