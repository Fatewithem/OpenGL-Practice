#version 410 core
layout (location = 0) in vec3 aQuadPos;   // (-0.5,-0.5,0) ... (0.5,0.5,0)

uniform mat4 u_ViewProj;
uniform vec3 u_Center;
uniform float u_Size;
uniform vec3 u_CamRight;
uniform vec3 u_CamUp;

out vec2 vUV;

void main()
{
    // Build world position from center + camera-aligned quad offsets
    vec3 worldOffset = u_CamRight * (aQuadPos.x * u_Size)
                     + u_CamUp    * (aQuadPos.y * u_Size);
    vec3 worldPos = u_Center + worldOffset;

    gl_Position = u_ViewProj * vec4(worldPos, 1.0);
    vUV = aQuadPos.xy + vec2(0.5); // map [-0.5,0.5] to [0,1]
}