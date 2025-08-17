file(GLOB_RECURSE FRAMEWORK_SRC ./*.cpp)

add_library(particle ${FRAMEWORK_SRC})

# 查找 glfw3 库
find_package(glfw3 REQUIRED)

# 确保找到 glfw3 的头文件路径
include_directories(${GLFW_INCLUDE_DIRS})

target_include_directories(particle PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_SOURCE_DIR}/glad/include
    /opt/homebrew/opt/glfw/include    # 添加 Homebrew 安装的 glfw 路径
) 

# 链接 glfw 库
target_link_libraries(particle PUBLIC glfw)