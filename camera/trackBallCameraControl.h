#pragma once

#include "cameraControl.h"

class TrackBallCameraControl : public CameraControl {
private:
    void pitch(float angle);
    void yaw(float angle);

private:
    float mMoveSpeed = 0.005f;

public:
    TrackBallCameraControl(/* args */);
    ~TrackBallCameraControl();

    // 父类函数是否需要重写
    void onCursor(double xpos, double ypos) override;

    void onScroll(float offset) override;
};


