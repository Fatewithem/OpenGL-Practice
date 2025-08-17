#include <iostream>

#define gl_app Application::getInstance()

class GLFWwindow;

using ResizeCallback = void(*) (int width, int height);
using KeyCallback = void(*) (int key, int scancode, int action, int mods);
using MouseCallback = void(*) (int button, int action, int mods);
using CursorCallback = void(*) (double xpos, double ypos);
using ScrollCallback = void(*) (double offset);

class Application {
public:
    static Application& getInstance();

    bool init(const int& width = 800, const int& height = 600);

    bool update();
    
    void destroy();

    uint32_t getWidth()const { return mWidth; }
    uint32_t getHeight()const {return mHeight; }

    void getCursorPosition(double* x, double* y);

    void setResizeCallback(ResizeCallback callback) {
        mResizeCallback = callback;
    }
    void setKeyCallback(KeyCallback callback) {
        mKeyCallback = callback;
    }
    void setMouseCallback(MouseCallback callback) {
        mMouseCallback = callback;
    }
    void setCursorCallback(CursorCallback callback) {
        mCursorCallback = callback;
    }
    void setScrollCallback(ScrollCallback callback) {
        mScrollCallback = callback;
    }

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

private:
    static void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xofffset, double yoffset);

private:
    uint32_t mWidth{ 0 };
    uint32_t mHeight{ 0 };
    GLFWwindow* mWindow { nullptr };

    ResizeCallback mResizeCallback { [](int,int){} };
    KeyCallback mKeyCallback { [](int,int,int,int){} };
    MouseCallback mMouseCallback { [](int,int,int){} };
    CursorCallback mCursorCallback { [](double,double){} };
    ScrollCallback mScrollCallback { [](double){} };

    Application();
    ~Application();
};