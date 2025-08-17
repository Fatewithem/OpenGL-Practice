#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform float threshold = 1.0;
uniform sampler2D bloomMask;

void main()
{
    vec3 color = texture(scene, TexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));  // 人眼感知亮度
    float mask = texture(bloomMask, TexCoords).r;

    // if (brightness > threshold && mask > 0.5)
    if (brightness > threshold)
        FragColor = vec4(color, 1.0);
    else
        FragColor = vec4(0.0);
}