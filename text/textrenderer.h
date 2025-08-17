#pragma once

#include <map>
#include <string>

#include <../core/core.h>
#include <../core/shader.h>

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
    GLuint Advance;     // Offset to advance to next glyph
};

class TextRenderer {
public:
    std::map<GLchar, Character> Characters;
    GLuint VAO, VBO;
    GLuint shaderID;

    TextRenderer();
    ~TextRenderer();

    bool init(const std::string& fontPath, GLuint width, GLuint height);
    void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color, Shader* shader);
};