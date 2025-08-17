#pragma once

#include "core.h"
#include <iostream>

class Geometry {
private:
    GLuint mVao { 0 };
    GLuint mPosVbo { 0 };
    GLuint mUvVbo { 0 };
    GLuint mNormalVbo { 0 };
    GLuint mEbo { 0 };

    // 参数需要传入三角形的数目
    uint32_t mIndicesCount{ 0 }; 

public:    
    Geometry();
    ~Geometry();

    static Geometry* createBox(float size);
    static Geometry* createSphere(float radius);
    static Geometry* createPlane(float size);
    static Geometry* createTriangle(float size);
    static Geometry* createRectangle(const std::vector<float>& positions, 
                                     const std::vector<float>& uvs);
    
    GLuint getVao() const { return mVao; }
    uint32_t getIndicesCount() const { return mIndicesCount; }
};