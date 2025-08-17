#include "checkerror.h"
#include <string>
#include <glad/glad.h>
#include <iostream>
#include <assert.h>

void checkerror() {
    GLenum errorCode = glGetError();
    std::string error = "";
    if(errorCode != GL_NO_ERROR) {
        switch(errorCode) {
            case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
            default:
                error = "UNKNOWN";
                break;
        }
        std::cout << error << std::endl;
        assert(false);
    }
}