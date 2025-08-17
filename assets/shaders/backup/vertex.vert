#version 410 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
layout(location=3) in vec3 offset;


out vec3 position;
out vec2 uv;
out vec3 normal;

out vec4 fragPosLightSpace; // 添加在顶点着色器输出
out mat3 TBN; // 法线贴图TBN空间

uniform float time;
uniform bool useTexture;

uniform mat4 transform;
uniform mat4 viewMatrix;
uniform mat4 pespectiveMatrix;

uniform mat4 lightSpaceMatrix; // 新增 uniform：光源空间变换矩阵

void main()
{
    vec4 worldPos = transform * vec4(aPos, 1.0);

    if(useTexture) {
        // vec4 pos = vec4(worldPos.x + offset.x, worldPos.y + offset.y, worldPos.z + offset.z, 1.0);
        // gl_Position = pespectiveMatrix * viewMatrix * pos;
        gl_Position = pespectiveMatrix * viewMatrix * worldPos; // MVP模式
    } else {
        gl_Position = pespectiveMatrix * viewMatrix * worldPos; // MVP模式
    }
     
    position = vec3(worldPos);

    normal = mat3(transpose(inverse(transform))) * aNormal;  // 法线变换

    // 构建TBN矩阵
    vec3 T = vec3(1.0, 0.0, 0.0);
    vec3 B = cross(normal, T);
    TBN = mat3(normalize(T), normalize(B), normalize(normal));

    uv = aUV;

    fragPosLightSpace = lightSpaceMatrix * worldPos;
} 