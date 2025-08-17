#include "cameraControl.h"

CameraControl::CameraControl(/* args */)
{
}

CameraControl::~CameraControl()
{
}

void CameraControl::onMouse(int button, int action, double xpos, double ypos) {
    bool pressed = action == GLFW_PRESS ? true : false;

    if(pressed) {
        mCurrentX = xpos;
        mCurrentY = ypos;
    }

    switch(button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        mLeftMouseDown = pressed;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        mRightMouseDown = pressed;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        mMiddleMouseDown = pressed;
        break;
    }
}
    
void CameraControl::onCursor(double xpos, double ypos) {
    
}
    
void CameraControl::onKey(int key, int action, int mods) {
    // 过滤repeat
    if(action == GLFW_REPEAT) {
        return;
    }

    // 检测按下或者抬起
    bool pressed = action == GLFW_PRESS ? true : false;

    // 记录keymap
    mKeyMap[key] = pressed;
}

void CameraControl::update() {

}

void CameraControl::setCamera(Camera* camera) { 
    mCamera = camera; 
}
    
void CameraControl::setSensitivity(float s) { 
    mSensitivity = s; 
}

void CameraControl::onScroll(float offset) {
    
}