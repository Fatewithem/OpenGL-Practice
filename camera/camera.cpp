#include "camera.h"


Camera::Camera(){

} 
Camera::~Camera(){

}

glm::mat4 Camera::getViewMatrix() {
    // lookat
    // eye center top
    glm::vec3 center = mPosition + mFront;

    return glm::lookAt(mPosition, center, mUp);
}

glm::mat4 Camera::getProjectionMatrix() {
    return glm::identity<glm::mat4>();
}

void Camera::scale(float deltaScale) {
    
}

void Camera::updateDirection() {
    mFront = glm::normalize(glm::cross(mUp, mRight));
}