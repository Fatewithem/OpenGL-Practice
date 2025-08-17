#version 410 core
out vec4 FragColor;

uniform vec3 u_Color;

void main()
{
    // 通过 fragment 的屏幕位置实现简单渐变淡出（可选）
    float dist = gl_FragCoord.x; // 或使用 gl_FragCoord.y / length(gl_FragCoord.xy)
    float alpha = 1.0; // 可加淡出渐变
    FragColor = vec4(u_Color, alpha);
}