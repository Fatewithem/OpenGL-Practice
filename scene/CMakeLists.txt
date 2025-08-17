file(GLOB_RECURSE FRAMEWORK_SRC ./*.cpp)

add_library(scene ${FRAMEWORK_SRC})

# 查找 glfw3 和 freetype 库
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

# 包含头文件路径
include_directories(${GLFW_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})

target_include_directories(scene PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_SOURCE_DIR}/glad/include
    /opt/homebrew/opt/glfw/include    # 添加 Homebrew 安装的 glfw 路径
) 

# 链接库
target_link_libraries(scene PUBLIC glfw ${FREETYPE_LIBRARIES})