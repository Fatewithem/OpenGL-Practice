#pragma once

#include "../../../core/core.h"
#include "../../../core/shader.h"

class Skybox {
public:
    Skybox(std::vector<std::string> faces);
    void draw(const glm::mat4 &view, const glm::mat4 &projection);

private:
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;
    Shader* skybox_shader;
    GLuint loadCubemap(std::vector<std::string> faces);
};