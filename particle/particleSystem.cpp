#include <cmath>
#include <iostream>
#include "particleSystem.h"

// [helpers] random in [0,1) and uniform direction on a unit sphere
static inline float frand01() { return rand() / (float)RAND_MAX; }

// 系统级循环时钟（8s 一次），供 update() / emit() 共用
static float gCycleClock = 0.0f;

static inline glm::vec3 randomUnitVector()
{
    // Uniform on sphere using spherical coordinates
    float z = frand01() * 2.0f - 1.0f;               // [-1, 1]
    float t = frand01() * 2.0f * static_cast<float>(M_PI);
    float r = sqrtf(glm::max(0.0f, 1.0f - z*z));
    return glm::vec3(r * cosf(t), z, r * sinf(t));
}

// Unit direction on XZ plane (ring emission)
static inline glm::vec3 randomUnitVectorXZ() {
    float t = frand01() * 2.0f * static_cast<float>(M_PI);
    return glm::vec3(cosf(t), 0.0f, sinf(t));
}

// 混合球面与平面方向：bias ∈ [0,1]，越大越贴近平面（1 = 纯平面）
static inline glm::vec3 biasedPlanarDirection(float bias) {
    bias = glm::clamp(bias, 0.0f, 1.0f);
    glm::vec3 dirP = randomUnitVectorXZ();
    glm::vec3 dirS = randomUnitVector();
    glm::vec3 d = glm::normalize(glm::mix(dirS, dirP, bias));
    return d;
}

// 平面偏向权重与早期竖直阻尼（单位：1/s）
static const float kPlanarBias   = 0.45f; // 0.75 → 大多数更贴近平面，但非零仰角
static const float kEarlyVyDamp  = 6.0f;  // 早期对 vy 的指数阻尼强度

// 粒子扩散控制参数
static const float kLifeSeconds = 7.0f;        // 生命周期：5s
static const float kRingNoGravityTime = 0.35f; // 平面扩散时间
static const float kMinSpeed = 2.0f;           // 初速度范围
static const float kMaxSpeed = 9.0f;          // 终止速度范围
static const float kGravityY = -6.0f;          // 平面期后重力
static const float kCycleSeconds = 8.0f;       // 粒子系统循环周期：8s

// 初始化整个粒子系统
void ParticleSystem::init() {
    particles.resize(1000); // 初始化粒子池
    setupMesh();            // set up quad
    for (auto& p : particles) {
        p.life = 0.0f;      // mark as dead so update() will respawn
        p.size = 0.1f;      // sensible default
        p.color = glm::vec4(1.0f);
        p.position = glm::vec3(0.0f);
        p.velocity = glm::vec3(0.0f);
    }
}

void ParticleSystem::update(float dt) {
    // 系统级循环时钟（与 emit() 共用）
    gCycleClock += dt;
    if (gCycleClock >= kCycleSeconds) gCycleClock -= kCycleSeconds; // 相当于 fmod

    // 可以在主循环中定期调用 emit(origin, count); 来不断生成粒子
    for (auto& p : particles) {
        if (p.life > 0.0f) {
            // age since spawn (life starts at 1.0f seconds)
            float age = kLifeSeconds - p.life;

            // age-based gravity ramp: 0 → 1 over [0, kRingNoGravityTime]
            float t = (kRingNoGravityTime > 0.0f) ? glm::clamp(age / kRingNoGravityTime, 0.0f, 1.0f) : 1.0f;
            float gScale = t * t * (3.0f - 2.0f * t); // smoothstep

            // Early phase: gently damp vertical velocity (don't hard clamp to 0)
            // v.y *= exp(-kEarlyVyDamp * dt) approximate discrete form
            p.velocity.y *= glm::max(0.0f, 1.0f - kEarlyVyDamp * dt);

            // Apply gravity scaled by ramp (kGravityY should be negative)
            p.velocity += glm::vec3(0.0f, kGravityY * gScale, 0.0f) * dt;

            // Integrate position after velocity update
            p.position += p.velocity * dt;

            // Lifetime decay
            p.life -= dt;
        }
        // 粒子重生机制：只在周期的前 5s 允许发射；后 3s 停止发射
        if (p.life <= 0.0f)
        {
            bool canSpawn = (gCycleClock < kLifeSeconds); // 0..5s 允许，5..8s 不发射
            if (!canSpawn) {
                continue; // 保持为 dead，等待下个周期
            }
            p.life = kLifeSeconds;
            p.position = glm::vec3(0.0f);
            {
                glm::vec3 dir = biasedPlanarDirection(kPlanarBias); // 偏向平面但非严格平面
                float speed = kMinSpeed + frand01() * (kMaxSpeed - kMinSpeed);
                p.velocity = dir * speed;
                p.color = glm::vec4(1.0f, 0.5f + frand01() * 0.5f, 0.0f, 1.0f);
                p.size = 0.08f + frand01() * 0.06f;
            }
        }
    }
}

void ParticleSystem::render(Shader* particleShader,
                            const glm::mat4& viewProj,
                            const glm::vec3& camRight,
                            const glm::vec3& camUp) {
    if (particles.empty()) return;

    // Shared uniforms
    particleShader->setMatrix4x4("u_ViewProj", viewProj);
    particleShader->setVector3("u_CamRight", camRight);
    particleShader->setVector3("u_CamUp",    camUp);

    // Shared color/switch uniforms
    particleShader->setVector3("u_GradA", glm::vec3(0.2f, 0.55f, 1.0f)); // blue
    particleShader->setVector3("u_GradB", glm::vec3(1.0f, 0.55f, 0.0f)); // orange
    particleShader->setFloat  ("u_LifeSeconds", kLifeSeconds);
    particleShader->setFloat  ("u_GradSpeed", 1.0f);
    
    glBindVertexArray(VAO);

    for (const Particle& p : particles) {
        if (p.life > 0.0f) {
            // Per-particle uniforms for billboard shaders
            particleShader->setVector3("u_Center", p.position);
            particleShader->setFloat  ("u_Size",   p.size);

            // glm::vec4 fadedColor = p.color;
            // fadedColor.a *= glm::clamp(p.life, 0.0f, 1.0f);
            // particleShader->setVector4("u_Color", fadedColor);

            // 片元里会用 u_Life / u_LifeSeconds 计算淡出，这里不再额外乘
            particleShader->setVector4("u_Color", p.color);
            particleShader->setFloat("u_Life", p.life);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    glBindVertexArray(0);
}

void ParticleSystem::emit(glm::vec3 origin, int count) {
    // 用户可以在主循环中周期性调用此函数实现持续发射
    for (auto& p : particles) {
        if (p.life <= 0.0f && count > 0) {
            // 与 update() 同步：仅在周期前 5s 允许发射；5..8s 不发射
            bool canSpawn = (gCycleClock < kLifeSeconds);
            if (!canSpawn) {
                continue; // 保持 dead，等下一个周期
            }
            p.position = origin;
            {
                glm::vec3 dir = biasedPlanarDirection(kPlanarBias);
                float speed = kMinSpeed + frand01() * (kMaxSpeed - kMinSpeed);
                p.velocity = dir * speed;
            }
            p.life = kLifeSeconds;
            p.size = 0.08f + frand01() * 0.06f;
            p.color = glm::vec4(1.0f);
            --count;
        }
    }
}

void ParticleSystem::setupMesh() {
    float quad[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}