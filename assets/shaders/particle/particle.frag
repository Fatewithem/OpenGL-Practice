#version 410 core
in vec2 vUV;
out vec4 FragColor;

uniform vec4  u_Color;        // 只用其中的 a 作为基准透明度
uniform float u_Life;         // 剩余寿命（秒）
uniform float u_LifeSeconds;  // 总寿命（秒）
uniform vec3  u_GradA;        // 渐变起始色（比如蓝）
uniform vec3  u_GradB;        // 渐变结束色（比如橙）
uniform vec3  u_Center;       // 每粒子中心（用于 hash 差异）
uniform float u_GradSpeed;    // 渐变加速系数：1=原速，>1 更快

// 小工具：根据中心位置做一个确定性“随机数”
float hash13(vec3 p) {
    // 经典小 hash：不追求高质量，只求稳定且足够分散
    float h = dot(p, vec3(12.9898, 78.233, 37.719));
    return fract(sin(h) * 43758.5453);
}

void main() {
    // 圆形软边
    float d = distance(vUV, vec2(0.5));
    float edge = smoothstep(0.52, 0.35, d);

    // 生命周期 0→1 的进度（0 = 刚出生，1 = 将消亡）
    float progress = 1.0 - clamp(u_Life / max(u_LifeSeconds, 1e-5), 0.0, 1.0);
    float speed = max(u_GradSpeed, 1.0);      // 未设置时(默认0)回退为1
    float fastProgress = clamp(progress * speed, 0.0, 1.0);

    // 渐变色：蓝 → 橙（也可以由 CPU 传入自定义颜色）
    vec3 base = mix(u_GradA, u_GradB, fastProgress);

    // 每粒子差异：用中心位置哈希一个亮度微调因子
    float jitter = hash13(u_Center * 3.1);    // 放大坐标避免过多重复
    float gain   = mix(0.85, 1.15, jitter);   // 亮度微调 ±15%
    vec3  rgb    = base * gain;

    // alpha 仍沿用 CPU 传入的 u_Color.a（里头你已乘过剩余寿命）
    float alpha  = u_Color.a * edge;

    if (alpha < 0.01) discard;
    FragColor = vec4(rgb, alpha);
}