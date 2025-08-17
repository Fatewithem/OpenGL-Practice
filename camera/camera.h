#pragma once

#include "../core/core.h"

class Camera {
public:
    Camera();
    ~Camera();

    glm::mat4 getViewMatrix();
    inline glm::vec3 getPosition() const { return mPosition; }
    inline glm::vec3 getFront() const { return mFront; }
    inline glm::vec3 getUp() const { return mUp; }
    inline glm::vec3 getRight() const { return mRight; }

    virtual glm::mat4 getProjectionMatrix();
    virtual void scale(float deltaScale);

public:
    glm::vec3 mPosition{0.0f , 0.0f, 5.0f};
    glm::vec3 mUp{0.0f, 1.0f, 0.0f };
    glm::vec3 mRight{1.0f, 0.0f, 0.0f};
    glm::vec3 mFront{0.0f, 0.0f, -1.0f};

    void updateDirection();
};