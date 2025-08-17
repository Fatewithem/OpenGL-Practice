#pragma once
#include "../core/core.h"

// 提供一个或多个轨迹函数
namespace Trajectory {
    glm::vec3 circularPath(float t, float radius = 5.0f);
    glm::vec3 sineWavePath(float t, float speed = 1.0f, float amplitude = 1.0f);
    glm::mat4 getModelMatrix(float t, const glm::vec3& pos);
    glm::mat4 getPicModelMatrix(float t, const glm::vec3& start, const glm::vec3& end, float arcHeight, float spinAngle, const glm::vec3& spinAxis);
}