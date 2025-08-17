#include "Application.h"
#include "glad/glad.h" 
#include <GLFW/glfw3.h>

// 新的线程安全 getInstance 实现
Application& Application::getInstance() {
    static Application instance;
    return instance;
}

bool Application::init(const int& width, const int& height) {
    mWidth = width;
    mHeight = height;
    
    // GLFW初始化
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // mac需求
    glfwWindowHint(GLFW_SAMPLES, 4);
    
    // 指针创建window
    mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", NULL, NULL);

    if(mWindow == nullptr) return false;

    // 获取实际 framebuffer 尺寸
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(mWindow, &fbWidth, &fbHeight);
    mWidth = fbWidth;
    mHeight = fbHeight;

    // 绑定
    glfwMakeContextCurrent(mWindow);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(mWindow, frameBufferSizeCallback);
    glfwSetKeyCallback(mWindow, keyCallback);
    glfwSetMouseButtonCallback(mWindow, mouseCallback);
    glfwSetCursorPosCallback(mWindow, cursorCallback);
    glfwSetScrollCallback(mWindow, scrollCallback);

    // this全局唯一对象
    glfwSetWindowUserPointer(mWindow, this);

    return true;
}

bool Application::update() {
    if(glfwWindowShouldClose(mWindow)) return false;

    // 接受并分发窗口消息
    glfwPollEvents();
    
    // 双缓存
    glfwSwapBuffers(mWindow);

    return true;
}

void Application::destroy() {
    // 清理
    glfwTerminate();
}

void Application::getCursorPosition(double* x, double* y) {
    glfwGetCursorPos(mWindow, x, y);
}

void Application::frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
    std::cout << width << "," << height << std::endl;
    Application* self = (Application*)glfwGetWindowUserPointer(window);
    self->mResizeCallback(width, height);
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Application* self = (Application*)glfwGetWindowUserPointer(window);
    self->mKeyCallback(key, scancode, action, mods);
}

void Application::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    Application* self = (Application*)glfwGetWindowUserPointer(window);
    self->mMouseCallback(button, action, mods);
}

void Application::cursorCallback(GLFWwindow* window, double xpos, double ypos) {
    Application* self = (Application*)glfwGetWindowUserPointer(window);
    self->mCursorCallback(xpos, ypos);
}

void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* self = (Application*)glfwGetWindowUserPointer(window);
    self->mScrollCallback(yoffset);
}

// 构造和解构
Application::Application() {

}

Application::~Application() {

}