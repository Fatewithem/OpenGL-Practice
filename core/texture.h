#pragma once
#include "core.h"
#include <string>

class SingleTexture {
public:
    SingleTexture(const std::string& path, unsigned int unit);
    ~SingleTexture();

    void bind(int unit);

    int getWidth() const {return mWidth; };
    int getHeight() const {return mHeight; };

private:
    GLuint mTexture{ 0 };
    int mWidth{ 0 };
    int mHeight{ 0 };
    unsigned int mUnit{ 0 };
};