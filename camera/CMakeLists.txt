file(GLOB_RECURSE APPLICATION_SRC ./*.cpp)

add_library(camera ${APPLICATION_SRC})

# 查找 glfw3 库
find_package(glfw3 REQUIRED)

# 确保找到 glfw3 的头文件路径
# include_directories(${GLFW_INCLUDE_DIRS})

# 添加 glad 的头文件路径
target_include_directories(camera PUBLIC
    ${CMAKE_SOURCE_DIR}/glad/include  # 确保 glad 的路径正确
    /opt/homebrew/opt/glfw/include    # 添加 Homebrew 安装的 glfw 路径
)

# 链接 glfw 库
target_link_libraries(camera PUBLIC glfw)