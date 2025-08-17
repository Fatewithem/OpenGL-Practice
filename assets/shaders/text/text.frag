#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;
uniform vec3 outlineColor;
uniform vec3 shadowColor;
uniform vec2 pixelSize;

void main() {
    float baseAlpha = texture(text, TexCoords).r;

    float outlineAlpha = 0.0;
    for (float dx = -1.0; dx <= 1.0; dx += 1.0)
    for (float dy = -1.0; dy <= 1.0; dy += 1.0) {
        vec2 offset = vec2(dx, dy) * pixelSize;
        float sample = texture(text, TexCoords + offset).r;
        outlineAlpha = max(outlineAlpha, sample);
    }

    float shadowAlpha = texture(text, TexCoords + pixelSize * vec2(4.0, -4.0)).r;

    vec4 shadowLayer  = vec4(shadowColor, shadowAlpha * 0.6);
    vec4 outlineLayer = vec4(outlineColor, max(0.0, outlineAlpha - baseAlpha) * 0.6);
    vec4 textLayer    = vec4(textColor, baseAlpha);

    // 更自然的阴影和描边合成
    vec4 result = vec4(0.0);
    result = mix(result, shadowLayer, shadowLayer.a);
    result = mix(result, outlineLayer, outlineLayer.a);
    result = mix(result, textLayer, textLayer.a);
    FragColor = result;

    if (FragColor.a <= 0.01)
        discard;
}