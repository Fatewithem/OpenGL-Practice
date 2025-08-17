#version 410 core
in vec3 TexCoords;
out vec4 FragColor;
uniform samplerCube skybox;

void main() {
    
    FragColor = texture(skybox, TexCoords);

    gl_FragDepth = 1.0;  // ✅ 关键补充
}