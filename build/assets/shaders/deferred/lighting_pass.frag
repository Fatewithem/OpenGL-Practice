#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionWorld;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D shadowMap;
uniform sampler2D ssao;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform mat4 lightSpaceMatrix;

uniform bool useTexture;

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

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos);
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 fragPos);

void main()
{
    vec3 FragPos = texture(gPositionWorld, TexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;

    // frag->view
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 lightDir = normalize(-dirLight.direction);

    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace, Normal, FragPos);

    float ao = texture(ssao, TexCoords).r;
    ao = pow(ao, 1.5); // 放大 AO 效果在远处的影响

    vec3 ambient = vec3(0.3) * ao;
    vec3 lightingColor = ambient + (1.0 - shadow) * CalcDirLight(dirLight, Normal, viewDir)
                                  + CalcPointLight(pointLight, Normal, viewDir, FragPos);

    // FragColor = vec4(lightingColor * Albedo, 1.0);
    
    vec3 finalColor = lightingColor * Albedo;
    finalColor = clamp(finalColor, 0.0, 1.0);
    FragColor = vec4(finalColor, 1.0);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

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
    float spec = pow(max(dot(halfwayDir, normal), 0.0), 32.0);

    vec3 ambient  = light.diffuse;
    vec3 diffuse  = diff * light.diffuse;
    vec3 specular = spec * light.specular;

    return ambient + diffuse + specular;
}

// 传入点光源struct，normal， 视角方向(从frag->view)，fragment的位置
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
    vec3 lightDir = normalize(light.position - fragPos);  // 光路方向，frag->light
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); // 衰减

    // 聚光影响
    // vec3 spotDir = normalize(fragPos - spotLight.position);
    // float theta = dot(spotDir, normalize(spotLight.direction));
    // float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    // float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);  

    float diff = max(dot(normal, lightDir), 0.0);

    // Phong：计算反射光与视角
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // Blinn Phong： 计算半程与法线
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 ambient  = light.diffuse * attenuation;
    vec3 diffuse  = diff * light.diffuse * attenuation; //  * intensity
    vec3 specular = spec * light.specular * attenuation; //  * intensity

    return ambient + diffuse + specular;
}

// 阴影计算
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 fragPos) {
    // 透视除法 + [0,1] 映射
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.x < 0.0 || projCoords.x > 1.0 ||
       projCoords.y < 0.0 || projCoords.y > 1.0 ||
       projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.01 * (1.0 - dot(normal, normalize(-dirLight.direction))), 0.003);
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

    return shadow;
}