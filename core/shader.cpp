#include "shader.h"

#include <glm/gtc/type_ptr.hpp>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
    std::string vertexCode, fragmentCode, geometryCode;
    std::ifstream vShaderFile, fShaderFile, gShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        if (geometryPath != nullptr) {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR: Shader File Error: " << e.what() << std::endl;
    }

    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);
    checkShaderErrors(vertex, "VERTEX");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);
    checkShaderErrors(fragment, "FRAGMENT");

    GLuint geometry = 0;
    if (!geometryCode.empty()) {
        const char* geometryShaderSource = geometryCode.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &geometryShaderSource, NULL);
        glCompileShader(geometry);
        checkShaderErrors(geometry, "GEOMETRY");
    }

    mProgram = glCreateProgram();
    glAttachShader(mProgram, vertex);
    glAttachShader(mProgram, fragment);
    if (geometry) glAttachShader(mProgram, geometry);
    glLinkProgram(mProgram);
    checkShaderErrors(mProgram, "LINK");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometry) glDeleteShader(geometry);
}

Shader::~Shader() {

}

// 绑定当前program以及释放
void Shader::begin(){
    glUseProgram(mProgram);
}
void Shader::end(){
    glUseProgram(0);
}

// 传参函数
void Shader::setFloat(const std::string& name, float value) {
    GLint location = glGetUniformLocation(mProgram, name.c_str());

    glUniform1f(location, value);
}

void Shader::setVector2(const std::string& name, float x, float y) {
    GLuint loaction = glGetUniformLocation(mProgram, name.c_str());

    glUniform2f(loaction, x, y);
}

void Shader::setVector2(const std::string& name, const glm::vec2& vec) {
    GLuint loaction = glGetUniformLocation(mProgram, name.c_str());

    glUniform2fv(loaction, 1, glm::value_ptr(vec));
}

void Shader::setVector3(const std::string& name, float x, float y, float z) {
    GLuint loaction = glGetUniformLocation(mProgram, name.c_str());

    glUniform3f(loaction, x, y, z);
}

void Shader::setVector3(const std::string& name, const float* values) {
    GLuint loaction = glGetUniformLocation(mProgram, name.c_str());

    glUniform3fv(loaction, 1, values);
}

void Shader::setVector3(const std::string& name, const glm::vec3& vec) {
    GLuint location = glGetUniformLocation(mProgram, name.c_str());
    
    glUniform3fv(location, 1, glm::value_ptr(vec));
}

void Shader::setVector4(const std::string& name, const glm::vec4& vec) {
    GLuint location = glGetUniformLocation(mProgram, name.c_str());
    
    glUniform4fv(location, 1, glm::value_ptr(vec));
}

void Shader::setInt(const std::string& name, int value) {
    GLuint loaction = glGetUniformLocation(mProgram, name.c_str());

    glUniform1i(loaction, value);
}

void Shader::setMatrix4x4(const std::string& name, glm::mat4 value) {
    GLuint loaction = glGetUniformLocation(mProgram, name.c_str());
    
    // tanspose参数(行优先还是列优先)
    glUniformMatrix4fv(loaction, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setBool(const std::string& name, bool value) {
    GLuint loaction = glGetUniformLocation(mProgram, name.c_str());

    glUniform1i(loaction, value);
}

void Shader::setKernel(const std::vector<glm::vec3>& kernel) {
    for (unsigned int i = 0; i < kernel.size(); ++i) {
        std::string uniformName = "samples[" + std::to_string(i) + "]";
        setVector3(uniformName.c_str(), kernel[i]);
    }
}