#pragma once

#include "../core/core.h"  
#include "../core/shader.h"

// 平行光
struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 diffuse;
    glm::vec3 specular;

    void apply(Shader* shader, const std::string& name = "dirLight") const {
        shader->setVector3(name + ".direction", direction);
        shader->setVector3(name + ".diffuse", diffuse);
        shader->setVector3(name + ".specular", specular);
    }
};

// 点光源
struct PointLight {
    glm::vec3 position;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    void apply(Shader* shader, const std::string& name = "pointLight") const {
        shader->setVector3(name + ".position", position);
        shader->setVector3(name + ".diffuse", diffuse);
        shader->setVector3(name + ".specular", specular);

        shader->setFloat(name + ".constant", constant);
        shader->setFloat(name + ".linear", linear);
        shader->setFloat(name + ".quadratic", quadratic);
    }
};

// 聚光
struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    void apply(Shader* shader, const std::string& name = "spotLight") const {
        shader->setVector3(name + ".position", position);
        shader->setVector3(name + ".direction", direction);
        shader->setFloat(name + ".cutOff", cutOff);
        shader->setFloat(name + ".outerCutOff", outerCutOff); // ✅ 添加
    }
};


























