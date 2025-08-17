#version 410 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionView;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];        // sample kernel
uniform mat4 projection;

const vec2 noiseScale = vec2(1600.0/4.0, 1200.0/4.0);  // 按你的 viewport 尺寸调整

void main() {
    vec3 fragPos = texture(gPositionView, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    // 构建 TBN 矩阵
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    float sampleDepth= 0.0;
    for(int i = 0; i < 64; ++i) {
        vec3 sampleVec = TBN * samples[i];
        sampleVec = fragPos + sampleVec * 1.0; // 使用 radius = 1.0

        vec4 offset = vec4(sampleVec, 1.0);
        offset = projection * offset;              // 观察 -> 裁剪空间
        offset.xyz /= offset.w;                    // 透视除法
        offset.xyz = offset.xyz * 0.5 + 0.5;       // NDC -> [0, 1]

        // 假设gPosition.w中保存了view-space的depth值
        sampleDepth = texture(gPositionView, offset.xy).w;

        float rangeCheck = smoothstep(0.0, 1.0, 1.0 / abs(fragPos.z - sampleDepth));
        if (sampleDepth >= sampleVec.z)
            occlusion += rangeCheck;
    }

    occlusion = 1.0 - (occlusion / 64.0);
    FragColor = occlusion;
}