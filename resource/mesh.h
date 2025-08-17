#pragma once

#include "../core/core.h" 
#include <string.h>
#include <vector>
#include "../core/shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 UVs;
};

struct Texture {
    unsigned int id;
    std::string type;
    aiString path;
};

class Mesh
{
private:
    unsigned int mVAO, mVBO, mEBO;
    void setupMesh();

public:
    std::vector<Vertex> mVertices;
    std::vector<unsigned int> mIndices;
    std::vector<Texture> mTextures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    ~Mesh();

    void draw(Shader& shader);
};

