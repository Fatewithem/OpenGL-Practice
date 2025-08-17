#pragma once

#include "../core/core.h"
#include "../core/shader.h"
#include "mesh.h"

class Model {
private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> texture_loaded;

    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
    unsigned int TextureFromFile(const char *path, const std::string &directory);

public:
    Model(char* path);
    ~Model();

    void draw(Shader &shader);
};