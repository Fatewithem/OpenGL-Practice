#include "input.h"
#include "../application/Application.h"
#include "../scene/scene.h"

// 你这里依然可以访问 g_cameraControl (在 scene.cpp 里定义了 static 变量)

extern GameCameraControl* g_cameraControl;  // 这里声明全局外部变量

void onResize(int width, int height) {
    glViewport(0, 0, width, height);
    std::cout << "OnResize" << std::endl;
}

void OnKey(int key, int scancode, int action, int mods) {
    if (g_cameraControl) g_cameraControl->onKey(key, action, mods);
}

void OnMouse(int button, int action, int mods) {
    double x, y;
    gl_app.getCursorPosition(&x, &y);
    if (g_cameraControl) g_cameraControl->onMouse(button, action, x, y);
}

void OnCursor(double xpos, double ypos) {
    if (g_cameraControl) g_cameraControl->onCursor(xpos, ypos);
}

void onScroll(double offset) {
    if (g_cameraControl) g_cameraControl->onScroll(offset);
}