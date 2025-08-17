#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int debugMode;

void main()
{
    if (debugMode == 0) {
        // gPosition 可视化
        vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
        FragColor = vec4(hdrColor * 0.1, 1.0); 
    } else if (debugMode == 1) {
        // gNormal 可视化
        vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
        FragColor = vec4(hdrColor * 0.5 + 0.5, 1.0);
    } else if (debugMode == 2) {
        // gAlbedoSpec 直接显示
        vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
        FragColor = vec4(hdrColor, 1.0);
    } else if (debugMode == 3) {
        
        float depth = texture(screenTexture, TexCoords).r;
        if (depth >= 0.9999)
            FragColor = vec4(0.0, 1.0, 0.0, 1.0); // 深度接近1时用纯红色
        else
            FragColor = vec4(vec3(depth), 1.0);
    } else {
        vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
        FragColor = vec4(hdrColor, 1.0);
    }
}