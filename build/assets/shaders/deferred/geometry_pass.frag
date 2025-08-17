#version 410 core

layout(location = 0) out vec4 gPositionWorld;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out vec4 gPositionView;
layout(location = 4) out vec3 gBloomMask;

in vec3 FragPos;
in vec3 FragPosView;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform float time;

uniform bool useNormalMap;
uniform bool isGlowing;
uniform bool u_ForwardColor; // true: forward transparent path writes color to location 0
uniform float u_Alpha;        // alpha for forward transparent path (0..1)

void main()
{
    // Sample base albedo first (shared by both paths)
    vec3 albedo = texture(texture_diffuse1, TexCoords).rgb;

    // Compute normal (object/tangent space handling)
    vec3 N;
    if (useNormalMap) {
        vec2 scrollUV = TexCoords + vec2(time * 0.05, time * 0.03);
        vec3 tangentNormal = texture(texture_normal1, scrollUV).rgb;
        tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
        N = normalize(tangentNormal);
    } else {
        N = normalize(Normal);
    }

    // Forward transparent path: write color to location 0 with alpha, skip G-Buffer outputs
    if (u_ForwardColor) {
        // In the forward path, the FBO attached at location 0 expects final color
        gPositionWorld = vec4(albedo, clamp(u_Alpha, 0.0, 1.0));
        // The following writes are ignored if the FBO has no attachments for these locations,
        // but we still zero them to be explicit.
        gNormal      = vec4(0.0);
        gAlbedoSpec  = vec4(0.0);
        gPositionView= vec4(0.0);
        gBloomMask   = isGlowing ? vec3(1.0) : vec3(0.0);
        return;
    }

    // Deferred G-Buffer path (opaque)
    gPositionWorld = vec4(FragPos, 1.0);
    gPositionView  = vec4(FragPosView, -FragPosView.z);
    gNormal        = vec4(N, 1.0);
    gAlbedoSpec.rgb = albedo;
    gAlbedoSpec.a   = 1.0; // specular/roughness placeholder for future use
    gBloomMask      = isGlowing ? vec3(1.0) : vec3(0.0);
}