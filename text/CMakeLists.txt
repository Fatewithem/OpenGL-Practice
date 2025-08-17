file(GLOB_RECURSE FRAMEWORK_SRC ./*.cpp)

add_library(text ${FRAMEWORK_SRC})

 # 查找 glfw3 和 freetype 库
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

# message(STATUS "FREETYPE_INCLUDE_DIRS = ${FREETYPE_INCLUDE_DIRS}")

# 确保找到头文件路径
include_directories(
    ${GLFW_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)

target_include_directories(text PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_SOURCE_DIR}/glad/include
    /opt/homebrew/opt/glfw/include
) 

# 链接库
target_link_libraries(text PUBLIC glfw ${FREETYPE_LIBRARIES})
target_include_directories(text PUBLIC /opt/homebrew/include/freetype2)

