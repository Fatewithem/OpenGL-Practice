#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

uniform float u_Thickness;   // 线宽，NDC 空间尺度（~0.001~0.1 之间）
uniform vec3  u_Color;

out vec3 vColor;

// 将输入的两点扩展为一个屏幕空间厚度的矩形（近似）
void main()
{
    // 输入裁剪空间坐标
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;

    // 转到 NDC（-1..1）以便计算屏幕方向
    vec2 ndc0 = (p0.xy / p0.w);
    vec2 ndc1 = (p1.xy / p1.w);

    // 方向与法线（NDC）
    vec2 dir  = normalize(ndc1 - ndc0);
    // 如果两点重合，避免 NaN
    if (length(dir) < 1e-6) dir = vec2(1.0, 0.0);
    vec2 nrm  = vec2(-dir.y, dir.x);

    // 厚度（NDC）。注意：这没有做 viewport 等距校正，
    // 如需像素级厚度，请在 C++ 里把 u_Thickness 转成 NDC 尺度再传入。
    float t = max(u_Thickness, 1e-6);
    vec2 off0 = nrm * t;
    vec2 off1 = nrm * t;

    // 组装四个裁剪空间顶点（把 NDC 偏移乘以 w 回到裁剪空间）
    vec4 p0a = p0; p0a.xy += off0 * p0.w;
    vec4 p0b = p0; p0b.xy -= off0 * p0.w;
    vec4 p1a = p1; p1a.xy += off1 * p1.w;
    vec4 p1b = p1; p1b.xy -= off1 * p1.w;

    vColor = u_Color;
    gl_Position = p0a; EmitVertex();
    gl_Position = p0b; EmitVertex();
    gl_Position = p1a; EmitVertex();
    gl_Position = p1b; EmitVertex();
    EndPrimitive();
}