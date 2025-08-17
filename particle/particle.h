#pragma once
#include "../core/core.h"

class Particle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life;
    float size;

    Particle();
    Particle(const glm::vec3& pos, const glm::vec3& vel, const glm::vec4& col, float lifeTime);
};