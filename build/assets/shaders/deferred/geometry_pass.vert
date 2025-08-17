#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

out vec3 FragPos;
out vec3 FragPosView;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec4 viewPos = viewMatrix * worldPos;
    FragPos = worldPos.xyz;
    FragPosView = viewPos.xyz;
    Normal = mat3(transpose(inverse(viewMatrix * model))) * aNormal;
    TexCoords = aUV;

    gl_Position = projectionMatrix * viewPos;
}