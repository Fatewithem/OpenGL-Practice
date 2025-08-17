#pragma once

#include "core.h"
#include <string>
#include <iostream>

class Shader {
public:
    // 读取 vertex 以及 fragment 的位置，支持可选的 geometry shader 路径
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    ~Shader();

    // 绑定 program 编号
    void begin();
    void end();

    // 设置传入的 uniform 类型的变量（name + 变量类型）
    void setFloat(const std::string& name, float value);
    void setVector2(const std::string& name, float x, float y);
    void setVector2(const std::string& name, const glm::vec2& vec);
    void setVector3(const std::string& name, float x, float y, float z);
    void setVector3(const std::string& name, const float* values);
    void setVector3(const std::string& name, const glm::vec3& vec);
    void setVector4(const std::string& name, const glm::vec4& vec);
    void setInt(const std::string& name, int value);
    void setMatrix4x4(const std::string& name, glm::mat4 value);
    void setBool(const std::string& name, bool value);
    void setKernel(const std::vector<glm::vec3>& kernal);

private:
    // shader program
    void checkShaderErrors(GLuint target, std::string type) {
        int success = 0;
        char infoLog[1024];
        if(type == "VERTEX") {
            glGetShaderiv(target, GL_COMPILE_STATUS, &success);
            if(!success) {
                glGetShaderInfoLog(target, 1024, NULL, infoLog);
                std::cout << "Vertex Shader Error:\n" << infoLog << std::endl;
            }
        } else if(type == "FRAGMENT") {
            glGetShaderiv(target, GL_COMPILE_STATUS, &success);
            if(!success) {
                glGetShaderInfoLog(target, 1024, NULL, infoLog);
                std::cout << "Fragment Shader Error:\n" << infoLog << std::endl;
            }
        } else if(type == "GEOMETRY") {
            glGetShaderiv(target, GL_COMPILE_STATUS, &success);
            if(!success) {
                glGetShaderInfoLog(target, 1024, NULL, infoLog);
                std::cout << "Geometry Shader Error:\n" << infoLog << std::endl;
            }
        } else if(type == "LINK") {
            glGetProgramiv(target, GL_LINK_STATUS, &success);
            if(!success) {
                glGetProgramInfoLog(target, 1024, NULL, infoLog);
                std::cout << "Program Link Error:\n" << infoLog << std::endl;
            }
        }
    }

public:
    GLuint mProgram{ 0 };
};