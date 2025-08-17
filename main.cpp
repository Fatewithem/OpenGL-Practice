#include "application/Application.h"
#include "scene/scene.h"
#include "input/input.h"

int main()
{
    if(!gl_app.init()) {
        return -1;
    }

    // 注册所有输入回调
    gl_app.setResizeCallback(onResize);
    gl_app.setKeyCallback(OnKey);
    gl_app.setMouseCallback(OnMouse);
    gl_app.setCursorCallback(OnCursor);
    gl_app.setScrollCallback(onScroll);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Scene scene;
    scene.init();

    while(gl_app.update()) {
        scene.update();
        scene.render();
    }

    scene.cleanup();
    gl_app.destroy();
    return 0;
}
