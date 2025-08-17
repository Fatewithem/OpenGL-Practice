#version 410 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoInput;

const float offset = 1.0 / 300.0;  // 可根据屏幕尺寸或FBO大小自适应调整

void main()
{
    float result = 0.0;
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // 左上
        vec2( 0.0f,    offset), // 上
        vec2( offset,  offset), // 右上
        vec2(-offset,  0.0f),   // 左
        vec2( 0.0f,    0.0f),   // 中心
        vec2( offset,  0.0f),   // 右
        vec2(-offset, -offset), // 左下
        vec2( 0.0f,   -offset), // 下
        vec2( offset, -offset)  // 右下
    );

    for(int i = 0; i < 9; i++)
    {
        result += texture(ssaoInput, TexCoords + offsets[i]).r;
    }

    FragColor = result / 9.0;
}