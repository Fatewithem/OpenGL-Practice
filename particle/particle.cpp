#include "particle.h"

Particle::Particle()
    : position(0.0f), velocity(0.0f), color(1.0f), life(0.0f) {}

Particle::Particle(const glm::vec3& pos, const glm::vec3& vel, const glm::vec4& col, float lifeTime)
    : position(pos), velocity(vel), color(col), life(lifeTime) {}
