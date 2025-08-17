cmake_minimum_required(VERSION 3.10)

project(OpenGLDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add_definitions(-DDEBUG)

file(GLOB ASSETS "./assets")
file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})

add_subdirectory(wrapper)
add_subdirectory(application)
# add_subdirectory(glframework)
add_subdirectory(camera)
add_subdirectory(core)
add_subdirectory(resource)
add_subdirectory(scene)
add_subdirectory(renderer)
add_subdirectory(input)
add_subdirectory(particle)
add_subdirectory(animation)
add_subdirectory(text)

include_directories(${CMAKE_SOURCE_DIR}/wrapper)
include_directories(${CMAKE_SOURCE_DIR}/application)
include_directories(${CMAKE_SOURCE_DIR}/camera)
include_directories(${CMAKE_SOURCE_DIR}/core)
include_directories(${CMAKE_SOURCE_DIR}/resource)
include_directories(${CMAKE_SOURCE_DIR}/scene)
include_directories(${CMAKE_SOURCE_DIR}/renderer)
include_directories(${CMAKE_SOURCE_DIR}/input)
include_directories(${CMAKE_SOURCE_DIR}/particle)
include_directories(${CMAKE_SOURCE_DIR}/animation)
include_directories(${CMAKE_SOURCE_DIR}/text)
# include_directories(${CMAKE_SOURCE_DIR}/glframework)
include_directories(${CMAKE_SOURCE_DIR}/thirdParty/include)

# 找到Assimp头文件
include_directories(/Users/wuchengyu/Downloads/Code/OpenGL/thirdParty/include/assimp/include)

include_directories(/opt/homebrew/include/freetype2)
include_directories(/opt/homebrew/include)

# 添加 glad 和 include 路径
include_directories(${CMAKE_SOURCE_DIR}/glad/include)

# 创建一个库，包含 glad 的源文件
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/glad/src/glad.c)

# 查找 OpenGL 和 GLFW
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

find_package(Freetype REQUIRED)

message(STATUS "FREETYPE_INCLUDE_DIRS = ${FREETYPE_INCLUDE_DIRS}")
message(STATUS "FREETYPE_INCLUDE_DIR_ft2build = ${FREETYPE_INCLUDE_DIR_ft2build}")

set(CMAKE_PREFIX_PATH "/usr/local")

# 创建可执行文件
add_executable(OpenGLDemo main.cpp)

target_include_directories(OpenGLDemo PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
    /opt/homebrew/include
)

# 链接 GLFW, OpenGL, 和 glad 
target_link_libraries(OpenGLDemo
    glfw
    OpenGL::GL
    glad
    wrapper
    application
    # glframework
    camera
    core
    renderer
    resource
    scene
    input
    particle
    animation
    text
    /Users/wuchengyu/Downloads/Code/OpenGL/thirdParty/include/assimp/lib/libassimp.a
    z
    ${FREETYPE_LIBRARIES}
)