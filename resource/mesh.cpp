#include "mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
    mVertices = vertices;
    mIndices = indices;
    mTextures = textures;

    setupMesh();
}

Mesh::~Mesh() {
    
}

void Mesh::setupMesh() {
    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mEBO);
    glGenVertexArrays(1, &mVAO);

    // 先绑定VAO
    glBindVertexArray(mVAO);

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), mVertices.data(), GL_STATIC_DRAW);  // 或者使用 &mVertices[0]
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(unsigned int), mIndices.data(), GL_STATIC_DRAW);

    // 顶点位置
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // 顶点法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // 顶点UV
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, UVs));

    glBindVertexArray(0);
}

void Mesh::draw(Shader& shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;

    for(unsigned int i = 0; i < mTextures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string name  = mTextures[i].type;
        std::string number;

        if(name == "texture_diffuse") {
            number = std::to_string(diffuseNr++);
        }
        else if(name == "texture_specular") {
            number = std::to_string(specularNr++);
        }

        shader.setInt((name + number).c_str(), i);

        // std::cout << "Binding texture: type=" << name << ", number=" << number 
        //   << ", id=" << mTextures[i].id << std::endl;

        glBindTexture(GL_TEXTURE_2D, mTextures[i].id);
    }

    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // 恢复到默认纹理单元
    glActiveTexture(GL_TEXTURE0);
}