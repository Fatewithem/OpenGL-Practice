file(GLOB_RECURSE WRAPPER_SRC ./*.cpp)

add_library(wrapper ${WRAPPER_SRC})

target_include_directories(wrapper PUBLIC
    ${CMAKE_SOURCE_DIR}/glad/include
)