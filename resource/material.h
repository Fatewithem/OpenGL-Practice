#pragma once

#include "../core/core.h"
#include "../core/shader.h"

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    
    void apply(Shader* shader, const std::string name = "material") const {
        shader->setVector3(name + ".ambient", ambient);
        shader->setVector3(name + ".diffuse", diffuse);
        shader->setVector3(name + ".specular", specular);
        shader->setFloat(name + ".shininess", shininess);
    }
};