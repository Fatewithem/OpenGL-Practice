#include <algorithm>
#include "trajectory.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// 插值两个不同轴和角度之间的旋转
static glm::mat4 interpolateRotation(const glm::vec3& axis1, float angle1, const glm::vec3& axis2, float angle2, float alpha) {
    glm::quat q1 = glm::angleAxis(glm::radians(angle1), glm::normalize(axis1));
    glm::quat q2 = glm::angleAxis(glm::radians(angle2), glm::normalize(axis2));
    glm::quat qInterp = glm::slerp(q1, q2, alpha);
    return glm::toMat4(qInterp);
}

// 辅助函数：将 alpha 分段映射为角度插值比例
static float interpolateAngleSplit(float alpha, float splitRatio = 0.9f) {
    if (alpha <= 0.5f) {
        float t = alpha / 0.5f;
        return splitRatio * t;
    } else {
        float t = (alpha - 0.5f) / 0.5f;
        return splitRatio + (1.0f - splitRatio) * t;
    }
}

glm::vec3 Trajectory::circularPath(float t, float radius) {
    return glm::vec3(radius * cos(t), 0.0f, radius * sin(t));
}

glm::vec3 Trajectory::sineWavePath(float t, float speed, float amplitude) {
    return glm::vec3(t, amplitude * sin(speed * t), 0.0f);
}

// 分为前进4s，维持1s，返回2s

glm::mat4 Trajectory::getModelMatrix(float t, const glm::vec3& pos) {
    glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 endPos   = glm::vec3(0.0f, 0.0f, -6.0f);
    
    float forwardDuration = 4.0f;
    float pauseDuration = 1.0f;
    float returnDuration = 2.0f;
    float totalCycle = forwardDuration + pauseDuration + returnDuration + pauseDuration;

    float timeInCycle = fmod(t, totalCycle);
    glm::vec3 interpolatedPos;

    if (timeInCycle <= forwardDuration) {
        float alpha = timeInCycle / forwardDuration;
        float angleAlpha = interpolateAngleSplit(alpha, 0.8f);
        interpolatedPos = glm::mix(startPos, endPos, angleAlpha);
    } else if (timeInCycle <= forwardDuration + pauseDuration) {
        interpolatedPos = endPos; // 停在终点
    } else if (timeInCycle <= forwardDuration + pauseDuration + returnDuration) {
        float beta = (timeInCycle - forwardDuration - pauseDuration) / returnDuration;
        float smoothBeta = sin(beta * glm::half_pi<float>());
        interpolatedPos = glm::mix(endPos, startPos, smoothBeta);
    } else {
        interpolatedPos = startPos; // 停在起点
    }

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), interpolatedPos);

    // 修改自转，前进时顺时针旋转，返回时逆时针旋转，回到初始位置角度为0
    float angle = 0.0f;
    float endAngle = 40.0f;
    glm::mat4 rotation;
    if (timeInCycle <= forwardDuration) {
        float alpha = timeInCycle / forwardDuration;
        float angleAlpha = interpolateAngleSplit(alpha, 0.8f);
        angle = glm::radians(endAngle) * angleAlpha;
        rotation = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f)));
    } else if (timeInCycle <= forwardDuration + pauseDuration) {
        angle = glm::radians(endAngle); // 保持终点角度
        rotation = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f)));
    } else if (timeInCycle <= forwardDuration + pauseDuration + returnDuration) {
        float beta = (timeInCycle - forwardDuration - pauseDuration) / returnDuration;
        float smoothBeta = sin(beta * glm::half_pi<float>());
        interpolatedPos = glm::mix(endPos, startPos, smoothBeta);
        translation = glm::translate(glm::mat4(1.0f), interpolatedPos);
        // 从 40° 的 (-1, -1, 0) 插值到 180° 的 Y 轴旋转
        rotation = interpolateRotation(glm::vec3(-1.0f, -1.0f, 0.0f), 40.0f, glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, beta);
    } else {
        angle = 0.0f; // 回到起点角度
        rotation = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f)));
    }

    return translation * rotation;
}

// 新增：为四个平面做弧线偏移和轻微旋转的函数
glm::mat4 Trajectory::getPicModelMatrix(float t, const glm::vec3& start, const glm::vec3& end, float arcHeight, float spinAngle, const glm::vec3& spinAxis) {
    // 与 getModelMatrix 对齐：前进4s → 停1s → 返回2s → 停1s（总 8s 循环）
    // 与 getModelMatrix 对齐：前进4s → 停1s → 返回2s → 停1s
    const float forwardDuration = 4.0f;
    const float pauseDuration   = 1.0f;
    const float returnDuration  = 2.0f;
    const float totalCycle      = forwardDuration + pauseDuration + returnDuration + pauseDuration; // 8s

    float timeInCycle = fmod(t, totalCycle);

    // 位置插值（保持与 getModelMatrix 一致的节奏：前进阶段用 interpolateAngleSplit 作为进度）
    glm::vec3 pos;
    float angle = 0.0f; // 自转角（弧度）

    if (timeInCycle <= forwardDuration) {
        // 前进4s：0→1 进度，前半段更快（9:1 分配）
        float alpha = timeInCycle / forwardDuration;              // 0→1
        float ease  = interpolateAngleSplit(alpha, 0.8f);         // 与 getModelMatrix 一致的节奏
        glm::vec3 linearPos = glm::mix(start, end, ease);
        float arcY = arcHeight * sin(glm::pi<float>() * ease);    // 弧线抬升
        pos = linearPos + glm::vec3(0.0f, arcY, 0.0f);
        angle = spinAngle * ease;                                 // 自转按同一节奏增长（弧度）
    } else if (timeInCycle <= forwardDuration + pauseDuration) {
        // 终点停1s
        pos = end;
        angle = spinAngle;                                        // 保持终点角度
    } else if (timeInCycle <= forwardDuration + pauseDuration + returnDuration) {
        // 返回2s：从 1 平滑回到 0（sin 平滑），并回到起点；旋转按原轴原角度回退到 0
        float beta = (timeInCycle - forwardDuration - pauseDuration) / returnDuration; // 0→1
        float smoothBeta = sin(beta * glm::half_pi<float>());      // 平滑
        pos = glm::mix(end, start, smoothBeta);
        angle = spinAngle * (1.0f - smoothBeta);                   // 原轴原角度回退
    } else {
        // 起点停1s
        pos = start;
        angle = 0.0f;                                             // 回到初始角度
    }

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), pos);
    glm::mat4 rotation    = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(spinAxis));

    return translation * rotation;
}