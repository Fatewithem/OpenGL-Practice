#version 330 core
out vec4 FragColor;

in vec2 uv;
in vec3 position;
in vec3 normal;
in vec4 fragPosLightSpace;
in mat3 TBN; // TBN

in vec3 stddd;

uniform vec3 uColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float time;
uniform bool useTexture;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform sampler2D shadowMap;

uniform sampler2D normalMap;
uniform bool useNormalMap;

// 设置材质
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
uniform Material material;

// 平行光源
struct DirectionalLight {
    vec3 direction;
    vec3 diffuse;
    vec3 specular;
}; 
uniform DirectionalLight dirLight;

// 点光源
struct PointLight {
    vec3 position;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
uniform PointLight pointLight;

// 聚光
struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
};
uniform SpotLight spotLight;

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos);
float ShadowCalculation(vec4 fragPosLightSpace, vec3 norm);

void main()
{
    vec3 norm;
    if(useNormalMap) {
        vec3 tangentNormal = texture(normalMap, uv).rgb;
        tangentNormal = tangentNormal * 2.0 - 1.0;
        norm = normalize(TBN * tangentNormal);
    } else {
        norm = normalize(normal);
    }

    vec3 viewDir = normalize(viewPos - position);

    // 设置光照
    vec3 result = vec3(0.0);
    result += CalcPointLight(pointLight, norm, viewDir, position);
    result += CalcDirLight(dirLight, norm, viewDir);

    // 设置阴影贴图
    float shadow = ShadowCalculation(fragPosLightSpace, norm);
 
    if(useTexture) {
        vec3 texColor = texture(texture_diffuse1, uv).rgb;

        // 计算完整光照
        vec3 lighting = result;

        // 将 ambient 与阴影解耦：ambient 不受阴影影响，diffuse/specular 受影响
        vec3 ambient = material.ambient * texColor;
        vec3 lightNoAmbient = lighting - ambient;
        vec3 shaded = (1.0 - shadow) * lightNoAmbient;

        vec3 finalColor = ambient + shaded;
        vec3 hdrColor = finalColor * texColor;
        vec3 mapped = hdrColor / (hdrColor + vec3(1.0)); // Reinhard tone mapping
    
        FragColor = vec4(mapped, 1.0);
    } else {
        FragColor = vec4(uColor * 2.0, 1.0);
    }
} 

// 传入点光源struct，normal， 视角方向(从frag->view)，fragment的位置
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
    vec3 lightDir = normalize(light.position - fragPos);  // 光路方向，frag->light
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); // 衰减

    // 聚光影响
    vec3 spotDir = normalize(fragPos - spotLight.position);
    float theta = dot(spotDir, normalize(spotLight.direction));
    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);  

    float diff = max(dot(normal, lightDir), 0.0);
    // Phong：计算反射光与视角
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Blinn Phong： 计算半程与法线
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    vec3 ambient  = material.ambient * light.diffuse * attenuation;
    vec3 diffuse  = diff * material.diffuse * light.diffuse * attenuation * intensity;
    vec3 specular = spec * material.specular * light.specular * attenuation * intensity;

    return ambient + diffuse + specular;
}

// 平行光struct, normal, 视角方向(从frag->view)
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);

    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0); 
    // 镜面反射
    // Phong：计算反射光与视角
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Blinn Phong： 计算半程与法线
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfwayDir, normal), 0.0), material.shininess);

    vec3 ambient  = material.ambient  * light.diffuse;
    vec3 diffuse  = material.diffuse  * diff * light.diffuse;
    vec3 specular = material.specular * spec * light.specular;

    return ambient + diffuse + specular;
}

// 设置阴影贴图
float ShadowCalculation(vec4 fragPosLightSpace, vec3 norm) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // 调试用：可视化光照空间的投影坐标映射为颜色
    // FragColor = vec4(projCoords.xy, 0.0, 1.0);
    // return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(norm, normalize(lightPos - position))), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if(projCoords.z > 1.0) shadow = 0.0;

    return shadow;
}
