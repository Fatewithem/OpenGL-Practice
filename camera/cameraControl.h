#pragma once

#include "../core/core.h"
#include "camera.h"
#include <map>

class CameraControl {
private:
    /* data */
public:
    CameraControl(/* args */);
    ~CameraControl();

    virtual void onMouse(int button, int action, double xpos, double ypos);
    virtual void onCursor(double xpos, double ypos);
    virtual void onKey(int key, int action, int mods);
    virtual void onScroll(float offset);

    // 每一帧更新
    virtual void update();

    void setCamera(Camera* camera);
    void setSensitivity(float s);
    void setScaleSpeed(float s) { mScaleSpeed = s; };

protected:
    // 鼠标案件状态
    bool mLeftMouseDown = false;
    bool mRightMouseDown = false;
    bool mMiddleMouseDown = false;

    // 鼠标位置
    float mCurrentX = 0.0f, mCurrentY = 0.0f;

    // 灵敏的
    float mSensitivity = 0.2f;

    float mScaleSpeed = 0.2f;

    // 记录键盘按下的状态
    std::map<int, bool> mKeyMap;

    // 存储当前控制的相机
    Camera* mCamera = nullptr;
};