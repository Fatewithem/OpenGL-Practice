#version 410 core
layout (triangles) in; // 输入图元是三角形
layout (line_strip, max_vertices = 6) out;

in vec3 position[];
in vec3 worldNormal[];

uniform mat4 viewMatrix;
uniform mat4 pespectiveMatrix;

void generate_normal_line(int index)
{
    vec4 worldPos = vec4(position[index], 1.0);
    vec3 normal = normalize(worldNormal[index]);

    gl_Position = pespectiveMatrix * viewMatrix * worldPos;
    EmitVertex();

    gl_Position = pespectiveMatrix * viewMatrix * (worldPos + vec4(normal * 0.2, 0.0)); // 法线方向延伸
    EmitVertex();

    EndPrimitive();
}

void main()
{
    generate_normal_line(0);
    generate_normal_line(1);
    generate_normal_line(2);
}