#version 330 core
layout(location = 0) out vec4 gNormal;

void main() {
    gNormal = vec4(1.0, 0.6, 0.0, 1.0);  // 固定输出红色法线
}