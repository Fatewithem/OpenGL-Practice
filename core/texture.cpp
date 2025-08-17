#include "texture.h"

#include "../application/stb_image.h"

SingleTexture::SingleTexture(const std::string& path, unsigned int unit) {
    // 读取图片
    int channels;

    mUnit = unit;

    // 反转y
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &mWidth, &mHeight, &channels, STBI_rgb_alpha);

    // 生成纹理 激活
    glGenTextures(1, &mTexture);
    glActiveTexture(GL_TEXTURE0 + mUnit);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // 传入数据 开辟显存
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // int width = mWidth, height = mHeight;
    // //遍历每个mipmap层级，填充每个级别的数据
    // for(int level = 0; true; ++ level) {
    //     glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    //     if(width == 1 && height == 1) {
    //         break;
    //     }

    //     width = width > 1 ? width / 2 : 1;
    //     height = height > 1 ? height / 2 : 1;

    // }


    // 自动mipmap
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

 
    // 释放数据
    stbi_image_free(data);

    // 纹理过滤方式x
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // GL_NEAREST 选择单个level最邻近采样
    // GL_NEAREST           
    // MINMAP_LINEAR
    // MINMAP_NEAREST  
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // 纹理包裹方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
    

SingleTexture::~SingleTexture(){
    if(mTexture != 0) {
        glDeleteTextures(1, &mTexture);
    }
};

void SingleTexture::bind(int unit) {
    mUnit = unit;  // 更新记录
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, mTexture);
}

