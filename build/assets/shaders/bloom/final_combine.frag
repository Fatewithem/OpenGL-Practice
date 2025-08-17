#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure = 1.0;

void main()
{
    const float gamma = 2.2;
    // vec3 hdrColor = texture(bloomBlur, TexCoords).rgb;
    vec3 hdrColor = texture(scene, TexCoords).rgb + texture(bloomBlur, TexCoords).rgb;
    // 曝光
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // Gamma 矫正
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(hdrColor, 1.0);
}