#include "trackBallCameraControl.h"


TrackBallCameraControl::TrackBallCameraControl(/* args */)
{
}

TrackBallCameraControl::~TrackBallCameraControl()
{
}

void TrackBallCameraControl::onCursor(double xpos, double ypos) {
    if(mLeftMouseDown) {
        float deltaX = (xpos - mCurrentX) * mSensitivity;
        float deltaY = (ypos - mCurrentY) * mSensitivity;

        pitch(-deltaY);
        yaw(-deltaX);

    }else if(mMiddleMouseDown) {
        float deltaX = (xpos - mCurrentX) * mMoveSpeed;
        float deltaY = (ypos - mCurrentY) * mMoveSpeed;

        mCamera->mPosition += mCamera->mPosition * deltaX;
        mCamera->mRight -= mCamera->mRight * deltaY;
    }

    mCurrentX = xpos;
    mCurrentY = ypos;
}

void TrackBallCameraControl::onScroll(float offset) {
    mCamera->scale(mScaleSpeed * offset);
}

void TrackBallCameraControl::pitch(float angle) {
    // 绕着mRight旋转
    auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), mCamera->mRight);

    // 影响up和位置
    mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f); // vec4->vec3
    mCamera->mPosition = mat * glm::vec4(mCamera->mPosition, 1.0f);
}

void TrackBallCameraControl::yaw(float angle) {
    // 绕着世界坐标y轴旋转
    auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    // 影响up和位置
    mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f); // vec4->vec3
    mCamera->mRight = mat * glm::vec4(mCamera->mRight, 0.0f);
    mCamera->mPosition = mat * glm::vec4(mCamera->mPosition, 1.0f);
}