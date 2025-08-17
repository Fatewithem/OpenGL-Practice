#include "scene.h"
#include "../application/Application.h"
#include <iostream>
#include <random>
#include <cmath>

// 全局变量用于其他代码访问
GameCameraControl* g_cameraControl = nullptr;

// 统一纹理槽枚举，避免冲突
enum TextureUnit {
    TEX_SANA         = 2,  // 模型主纹理
    TEX_FLOOR        = 3,  // 地板贴图
    TEX_DEPTHMAP     = 4,  // 用于阴影贴图
    TEX_SKYBOX       = 5,
    TEX_NORMAL       = 6,
    NORMAL_WATER    = 8,
    TEX_BRIGHT_EXTRACT = 7,
};

// 统一方向光方向
glm::vec3 lightDir = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.5f));

void Scene::init() {
    prepareShaders();
    prepareGeometry();
    prepareGBuffer();
    prepareScreenFBO();
    prepareFullscreen();
    prepareCamera();
    prepareTexture();
    prepareSkybox();
    prepareShadowMap();
    prepareSSAO();
    prepareBloom();
    prepareText();

    particleSystem = new ParticleSystem();
    particleSystem->init();
}

// 主更新部分
void Scene::update() {
    // 预留未来输入、动画、逻辑更新
    g_cameraControl->update();
    
        // 粒子系统每帧更新和发射粒子
    if (particleSystem) {
        float timeNow = glfwGetTime();
        static float lastTime = timeNow;
        float dt = timeNow - lastTime;
        lastTime = timeNow;
        
        particleSystem->emit(glm::vec3(0.0f, 0.0f, 0.0f), 5);
        particleSystem->update(dt);
    }
}

// 主渲染部分
void Scene::render() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glDisable(GL_CULL_FACE);  // 禁止剔除，确保所有面可见

    renderGeometryPass();

    // Shadow/SSAO on GBuffer before lighting
    renderShadowMapPass();
    renderSSAOPass();

    // Deferred lighting into screenFBO
    renderLightingPass();

    // Blit depth from GBuffer to screenFBO (so skybox and forward transparent can depth-test against the opaque scene)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFBO);
    glBlitFramebuffer(0, 0, gl_app.getWidth(), gl_app.getHeight(), 0, 0, gl_app.getWidth(), gl_app.getHeight(),
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    // Draw skybox first (uses depth test, no depth writes)
    renderSkybox();

    // Then draw forward blended transparent pieces on top of the skybox
    renderPiecesPass();

    renderGlowObjects();   
    
    renderParticlePass();

    renderBrightExtractPass();

    renderBloomBlurPass();

    renderBloomCompositePass();

    

    // renderText();
}

// 清理资源
void Scene::cleanup() {
    delete geometryPassShader;
    delete lightingPassShader;
    delete geometry;
    delete camera;
    delete particleSystem;
    particleSystem = nullptr;
}

// 设置Shader
void Scene::prepareShaders() {
    geometryPassShader = new Shader(
        "assets/shaders/deferred/geometry_pass.vert",
        "assets/shaders/deferred/geometry_pass.frag");

    lightingPassShader = new Shader(
        "assets/shaders/deferred/lighting_pass.vert",
        "assets/shaders/deferred/lighting_pass.frag");

    screenShader = new Shader(
        "assets/shaders/v_passthrough.vert",
        "assets/shaders/f_passthrough.frag");

    // 阴影贴图
    depthShader = new Shader(
        "assets/shaders/shadowmap/depth.vert", 
        "assets/shaders/shadowmap/depth.frag");

    ssaoShader = new Shader(
        "assets/shaders/ssao/ssao.vert",
        "assets/shaders/ssao/ssao.frag");

    ssaoBlurShader = new Shader(
        "assets/shaders/ssao/ssao_blur.vert",
        "assets/shaders/ssao/ssao_blur.frag");

    particleShader = new Shader(
        "assets/shaders/particle/particle.vert",
        "assets/shaders/particle/particle.frag");

    glowShader = new Shader(
        "assets/shaders/glow/glow.vert",
        "assets/shaders/glow/glow.frag",
        "assets/shaders/glow/glow_line.geom");

    brightExtractShader = new Shader(
        "assets/shaders/bloom/bright_extract.vert",
        "assets/shaders/bloom/bright_extract.frag");

    blurShader = new Shader(
        "assets/shaders/bloom/blur.vert", 
        "assets/shaders/bloom/blur.frag");

    finalCompositeShader = new Shader(
        "assets/shaders/bloom/final_combine.vert",
        "assets/shaders/bloom/final_combine.frag");

    textShader = new Shader(
        "assets/shaders/text/text.vert",
        "assets/shaders/text/text.frag");
}

// 设置几何vao
void Scene::prepareGeometry() {
    // 主体box
    geometry = Geometry::createBox(3.0f);
    // 地板
    floorGeometry = Geometry::createPlane(20.0f);
    // 碎片效果
    piecesGeometry = Geometry::createPlane(3.0f);
    // 发光三角形
    triangleGeometry = Geometry::createTriangle(2.0f);
    // 图片碎片
    {
        std::vector<std::vector<float>> verticesList = {
            // 每组是一个矩形，顺时针 4 个点，每点 3 个 float (x, y, z)
            {
                -0.5f, -0.5f, 0.0f,   0.5f, -0.5f, 0.0f,
                 0.5f,  0.5f, 0.0f,  -0.5f,  0.5f, 0.0f
            },
            {
                -1.0f, -0.2f, 0.0f,   1.0f, -0.2f, 0.0f,
                1.0f,  0.2f, 0.0f,  -1.0f,  0.2f, 0.0f
            },
            {
                -0.3f, -0.8f, 0.0f,   0.3f, -0.8f, 0.0f,
                 0.3f, -0.2f, 0.0f,  -0.3f, -0.2f, 0.0f
            }
        };
        std::vector<std::vector<float>> uvsList = {
            {
                0.0f, 0.0f,  // 左下
                1.0f, 0.0f,  // 右下
                1.0f, 1.0f,  // 右上
                0.0f, 1.0f   // 左上
            },
            {
                0.0f, 0.0f,   // 左下
                1.0f, 0.0f,   // 右下
                1.0f, 0.5f,   // 右中
                0.0f, 0.5f    // 左中
            },
            {
                0.0f, 0.5f,   // 左中
                1.0f, 0.5f,   // 右中
                1.0f, 1.0f,   // 右上
                0.0f, 1.0f    // 左上
            }
        };
        rectangleGeometry.resize(3); 
        for (int i = 0; i < 3; ++i) {
            rectangleGeometry[i] = Geometry::createRectangle(verticesList[i], uvsList[i]);
        }
    }
    // 四张正方形图片
    {
        std::vector<float> verticesList = {
            -1.5f, -1.5f, 0.0f,
             1.5f, -1.5f, 0.0f,
             1.5f,  1.5f, 0.0f,
            -1.5f,  1.5f, 0.0f
        };
        std::vector<float> uvsList = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };
        squareGeometry.resize(4); 
        for (int i = 0; i < 4; ++i) {
            squareGeometry[i] = Geometry::createRectangle(verticesList, uvsList);
        }
    }    
}

// 设置纹理
void Scene::prepareTexture() {
    // 纹理贴图
    texture_sana = new SingleTexture("assets/textures/sana.png", TEX_SANA);
    texture_floor = new SingleTexture("assets/textures/floor.png", TEX_FLOOR);

    // 法向贴图
    normal_water = new SingleTexture("assets/normals/water_normal.png", NORMAL_WATER);
}

// 设置GBuffer
void Scene::prepareGBuffer() {
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gl_app.getWidth(), gl_app.getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gl_app.getWidth(), gl_app.getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_app.getWidth(), gl_app.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // 新增view-space position G-buffer纹理
    glGenTextures(1, &gPositionView);
    glBindTexture(GL_TEXTURE_2D, gPositionView);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gl_app.getWidth(), gl_app.getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gPositionView, 0);

    glGenTextures(1, &gBloomMask);
    glBindTexture(GL_TEXTURE_2D, gBloomMask);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, gl_app.getWidth(), gl_app.getHeight(), 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gBloomMask, 0);

    GLuint attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
    glDrawBuffers(5, attachments);

    glGenTextures(1, &gDepth);
    glBindTexture(GL_TEXTURE_2D, gDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, gl_app.getWidth(), gl_app.getHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "GBuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::prepareScreenFBO() {
    glGenFramebuffers(1, &screenFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

    glGenTextures(1, &screenColorTex);
    glBindTexture(GL_TEXTURE_2D, screenColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gl_app.getWidth(), gl_app.getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColorTex, 0);

    glGenTextures(1, &screenDepth);
    glBindTexture(GL_TEXTURE_2D, screenDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, gl_app.getWidth(), gl_app.getHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, screenDepth, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Screen FBO not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 设置最后渲染的屏幕
void Scene::prepareFullscreen() {
    float quadVertices[] = {
        // positions     // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

// 设置相机
void Scene::prepareCamera() {
    camera = new PerspectiveCamera(60.0f, (float)gl_app.getWidth() / (float)gl_app.getHeight(), 0.1f, 1000.0f);

    cameraControl = new GameCameraControl();
    cameraControl->setCamera(camera);

    // 将当前cameraControl赋值给全局
    g_cameraControl = cameraControl;
}

// 设置天空盒
void Scene::prepareSkybox() {
    std::vector<std::string> faces = {
        "assets/skybox/right.bmp",
        "assets/skybox/left.bmp",
        "assets/skybox/top.bmp",
        "assets/skybox/bottom.bmp",
        "assets/skybox/front.bmp",
        "assets/skybox/back.bmp"
    };
    skybox = new Skybox(faces);
}

// 设置阴影贴图
void Scene::prepareShadowMap() {
    glGenFramebuffers(1, &shadowFBO);

    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, 
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::prepareSSAO() {
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gl_app.getWidth(), gl_app.getHeight(), 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 创建SSAO采样核
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i) {
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;
        scale = glm::mix(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // 创建旋转噪声纹理
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f);
        ssaoNoise.push_back(noise);
    }

    glGenTextures(1, &noiseTex);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 创建 SSAO 模糊帧缓冲
    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

    glGenTextures(1, &ssaoBlurColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gl_app.getWidth(), gl_app.getHeight(), 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurColorBuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::prepareBloom() {
    // Bright Extract
    glGenFramebuffers(1, &bloomBrightFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, bloomBrightFBO);

    glGenTextures(1, &bloomBrightTex);
    glBindTexture(GL_TEXTURE_2D, bloomBrightTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gl_app.getWidth(), gl_app.getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomBrightTex, 0);

    // Attach depth buffer to bloomBrightFBO
    glGenRenderbuffers(1, &bloomExtractDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, bloomExtractDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, gl_app.getWidth(), gl_app.getHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bloomExtractDepthRBO);

    // Optionally check framebuffer completeness
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Bright FBO not complete!" << std::endl;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Blur
    glGenFramebuffers(2, bloomPingpongFBO);
    glGenTextures(2, bloomPingpongTex);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, bloomPingpongFBO[i]);

        glBindTexture(GL_TEXTURE_2D, bloomPingpongTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gl_app.getWidth(), gl_app.getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomPingpongTex[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Pingpong FBO " << i << " not complete!" << std::endl;
    }   
    glBindFramebuffer(GL_FRAMEBUFFER, 0);   
}

void Scene::prepareText() {
    textRenderer = new TextRenderer();
    textRenderer->init("../assets/font/cute.ttf", gl_app.getWidth(), gl_app.getHeight());
}

void Scene::renderGeometryPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    geometryPassShader->begin();
    geometryPassShader->setMatrix4x4("viewMatrix", camera->getViewMatrix());
    geometryPassShader->setMatrix4x4("projectionMatrix", camera->getProjectionMatrix());
    
    float time = glfwGetTime();

    // 渲染中心box
    glBindVertexArray(geometry->getVao());
    // glm::vec3 pos_box = Trajectory::circularPath(time, 4.0f);
    glm::mat4 model_box = Trajectory::getModelMatrix(time, glm::vec3(0.0f));

    // glm::mat4 model_box = glm::mat4(1.0f);
    geometryPassShader->setMatrix4x4("model", model_box);
    geometryPassShader->setInt("texture_diffuse1", TEX_SANA);
    texture_sana->bind(TEX_SANA);
    geometryPassShader->setBool("useNormalMap", true);
    geometryPassShader->setBool("isGlowing", false);
    geometryPassShader->setBool("u_ForwardColor", false);
    geometryPassShader->setInt("texture_normal1", NORMAL_WATER);
    normal_water->bind(NORMAL_WATER);
    glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // 渲染地板
    // glBindVertexArray(floorGeometry->getVao());
    // glm::mat4 transformFloor = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.55f, 0.0f));
    // geometryPassShader->setMatrix4x4("model", transformFloor);
    // geometryPassShader->setInt("texture_diffuse1", TEX_FLOOR);
    // texture_floor->bind(TEX_FLOOR);
    // geometryPassShader->setBool("useNormalMap", false);
    // geometryPassShader->setBool("isGlowing", false);
    // glDrawElements(GL_TRIANGLES, floorGeometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
    // glBindVertexArray(0);

    // 渲染发光三角形
    // glBindVertexArray(triangleGeometry->getVao());
    // glm::mat4 triangle_model = glm::mat4(1.0f); // 可修改为其他变换
    // geometryPassShader->setMatrix4x4("model", triangle_model);
    // geometryPassShader->setBool("useNormalMap", false);
    // geometryPassShader->setBool("isGlowing", false);
    // glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);  // triangleGeometry->getIndicesCount()
    // glBindVertexArray(0);

    // 渲染四周图片；顺序：右上，左上，右下，左下
    glm::vec3 picStarts[4] = {
        glm::vec3(3.0, 1.5, 0.0), glm::vec3(-3.0, 1.5, 0.0), glm::vec3(3.0,-3.0, 0.0), glm::vec3(-3.0, -3.0, 0.0)
    };
    glm::vec3 picEnds[4] = {
        glm::vec3(4.0, 2.5, -5.0), glm::vec3(-4.0, 2.5, -5.0), glm::vec3(4.0,-4.0, -5.0), glm::vec3(-4.0, -4.0, -5.0)
    };
    glm::vec3 spins[4] = {
        glm::vec3(0.5, -1.0, 0.0), glm::vec3(-0.3, 1.0, 0.3), glm::vec3(0.1, -1.0, -0.4), glm::vec3(0.3, 1.0, 0.25)
    };

    for (int i = 0; i < 4; ++i) {
        glm::mat4 square_model = Trajectory::getPicModelMatrix(
            time,
            picStarts[i],           
            picEnds[i],       
            0.0f,             
            glm::radians(20.0f), 
            spins[i]
        );

        glBindVertexArray(squareGeometry[i]->getVao());
        geometryPassShader->setBool("u_ForwardColor", false);
        geometryPassShader->setMatrix4x4("model", square_model);
        geometryPassShader->setInt("texture_diffuse1", TEX_SANA);
        texture_sana->bind(TEX_SANA);

        glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    geometryPassShader->end();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::renderShadowMapPass() {
    float near = 1.0f, far = 20.0f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near, far);
    glm::mat4 lightView = glm::lookAt(-lightDir * 10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;

    depthShader->begin();
    depthShader->setMatrix4x4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);  // 确保写入深度缓冲区
    glDisable(GL_CULL_FACE);  // 禁止剔除，确保所有面可见

    // 渲染中心 box
    glBindVertexArray(geometry->getVao());
    glm::mat4 model_box = glm::mat4(1.0f);
    depthShader->setMatrix4x4("model", model_box);
    glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);

    // --- 也将四周图片与行星带碎片写入 ShadowMap ---
    float time = glfwGetTime();

    // A) 四周图片（与 geometry pass 同步）
    {
        glm::vec3 picStarts[4] = {
            glm::vec3(3.0, 1.5, 0.0), glm::vec3(-3.0, 1.5, 0.0), glm::vec3(3.0,-3.0, 0.0), glm::vec3(-3.0, -3.0, 0.0)
        };
        glm::vec3 picEnds[4] = {
            glm::vec3(4.0, 2.5, -5.0), glm::vec3(-4.0, 2.5, -5.0), glm::vec3(4.0,-4.0, -5.0), glm::vec3(-4.0, -4.0, -5.0)
        };
        glm::vec3 spins[4] = {
            glm::vec3(0.5, -1.0, 0.0), glm::vec3(-0.3, 1.0, 0.3), glm::vec3(0.1, -1.0, -0.4), glm::vec3(0.3, 1.0, 0.25)
        };
        for (int i = 0; i < 4; ++i) {
            glm::mat4 square_model = Trajectory::getPicModelMatrix(
                time,
                picStarts[i],
                picEnds[i],
                0.0f,
                glm::radians(20.0f),
                spins[i]
            );
            glBindVertexArray(squareGeometry[i]->getVao());
            depthShader->setMatrix4x4("model", square_model);
            glDrawElements(GL_TRIANGLES, squareGeometry[i]->getIndicesCount(), GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
    }

    // B) 行星带碎片（透明体的几何仍可写入深度阴影）
    {
        // 构建与 renderPiecesPass 相同的 tile 几何与随机种子（仅构建一次）
        struct TileGeom { Geometry* geo; float a0, a1, b0, b1; };
        static std::vector<TileGeom> s_tiles_shadow;
        static bool s_built_shadow = false;
        const int COLS = 8;
        const int ROWS = 6;
        if (!s_built_shadow) {
            s_tiles_shadow.reserve(COLS * ROWS);
            std::mt19937 rng(1337u);
            std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
            const std::vector<float> baseVerts = { -0.5f,-0.5f,0.0f,  0.5f,-0.5f,0.0f,  0.5f, 0.5f,0.0f, -0.5f, 0.5f,0.0f };
            for (int r = 0; r < ROWS; ++r) {
                for (int c = 0; c < COLS; ++c) {
                    float u0 = (float)c / (float)COLS;
                    float v0 = (float)r / (float)ROWS;
                    float u1 = (float)(c + 1) / (float)COLS;
                    float v1 = (float)(r + 1) / (float)ROWS;
                    std::vector<float> uvs = { u0,v0,  u1,v0,  u1,v1,  u0,v1 };
                    Geometry* g = Geometry::createRectangle(baseVerts, uvs);
                    s_tiles_shadow.push_back({ g, dist01(rng), dist01(rng), dist01(rng), dist01(rng) });
                }
            }
            s_built_shadow = true;
        }

        // 圆环/扰动参数（与 renderPiecesPass 对齐，以保持阴影同步）
        const float innerR = 7.0f;
        const float outerR = 9.0f;
        const float angularSpeed = 0.05f;
        const float ringTiltDeg = 7.5f;
        const float splitGap = 0.015f;
        const float triScale = 0.6f;
        const float jitterYMul = 2.8f;

        glm::mat4 ringTiltM = glm::rotate(glm::mat4(1.0f), glm::radians(ringTiltDeg), glm::vec3(1,0,0));
        auto ringPos = [&](float radius, float theta){ glm::vec4 p(radius * cosf(theta), 0.0f, radius * sinf(theta), 1.0f); p = ringTiltM * p; return glm::vec3(p); };

        int idx = 0;
        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                const TileGeom& tg = s_tiles_shadow[idx];
                float thetaA = tg.a0 * glm::two_pi<float>() + angularSpeed * time;
                float radiusA = glm::mix(innerR, outerR, tg.a1);
                float thetaB = tg.b0 * glm::two_pi<float>() - angularSpeed * 0.85f * time;
                float radiusB = glm::mix(innerR, outerR, tg.b1);

                float jitterXA = 0.06f * sinf(time * 1.37f + (r * 13 + c) * 0.71f + 0.5f);
                float jitterYA = 0.06f * cosf(time * 1.11f + (r * 17 + c) * 0.63f + 1.3f) * jitterYMul;
                float spinA    = 0.8f  * sinf(time * 0.97f + (r * 7  + c) * 0.47f + 0.2f);

                float jitterXB = 0.06f * sinf(time * 1.53f + (r * 19 + c) * 0.59f + 2.1f);
                float jitterYB = 0.06f * cosf(time * 1.27f + (r * 11 + c) * 0.67f + 0.9f) * jitterYMul;
                float spinB    = 0.8f  * cosf(time * 1.03f + (r * 5  + c) * 0.51f + 0.6f);

                glm::vec3 posA = ringPos(radiusA, thetaA);
                glm::vec3 posB = ringPos(radiusB, thetaB);

                // 朝向相机（此处用近似 yaw，阴影差别不大）
                auto faceCameraYaw = [&](const glm::vec3& pos){ glm::vec3 toCam = glm::normalize(camera->mPosition - pos); return atan2f(toCam.x, toCam.z); };
                float yawA = faceCameraYaw(posA);
                float yawB = faceCameraYaw(posB);

                // A
                {
                    glm::mat4 M(1.0f);
                    M = glm::translate(M, posA + glm::vec3(jitterXA, jitterYA * 0.5f, 0.0f));
                    M = glm::rotate(M, yawA, glm::vec3(0,1,0));
                    M = glm::rotate(M, spinA, glm::vec3(0,0,1));
                    M = glm::translate(M, glm::vec3(+splitGap, -splitGap, 0.0f));
                    M = glm::scale(M, glm::vec3(triScale, triScale, 1.0f));
                    glBindVertexArray(s_tiles_shadow[idx].geo->getVao());
                    depthShader->setMatrix4x4("model", M);
                    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)0);
                }
                // B
                {
                    glm::mat4 M(1.0f);
                    M = glm::translate(M, posB + glm::vec3(jitterXB, jitterYB * 0.5f, 0.0f));
                    M = glm::rotate(M, yawB, glm::vec3(0,1,0));
                    M = glm::rotate(M, spinB, glm::vec3(0,0,1));
                    M = glm::translate(M, glm::vec3(-splitGap, +splitGap, 0.0f));
                    M = glm::scale(M, glm::vec3(triScale, triScale, 1.0f));
                    glBindVertexArray(s_tiles_shadow[idx].geo->getVao());
                    depthShader->setMatrix4x4("model", M);
                    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * sizeof(unsigned int)));
                }

                // 可选：如果你也想让 clone A'/B' 投影，可复制上面两段并应用 dTheta/dRad 变化
                ++idx;
            }
        }
        glBindVertexArray(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    depthShader->end();

    // 恢复视口
    glViewport(0, 0, gl_app.getWidth(), gl_app.getHeight());
}

void Scene::renderSSAOPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);  // 输出到 SSAO 纹理

    ssaoShader->begin();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPositionView);
    ssaoShader->setInt("gPositionView", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    ssaoShader->setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTex);  // 小噪声纹理，用于旋转采样核
    ssaoShader->setInt("texNoise", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gDepth);  // 小噪声纹理，用于旋转采样核
    ssaoShader->setInt("gDepth", 3);

    ssaoShader->setMatrix4x4("projection", camera->getProjectionMatrix());
    ssaoShader->setKernel(ssaoKernel);  // 发送采样核数组

    glBindVertexArray(quadVAO);           // 渲染全屏矩形
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    ssaoShader->end();

    // Blur SSAO
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    ssaoBlurShader->begin();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    ssaoBlurShader->setInt("ssaoInput", 0);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    ssaoBlurShader->end();

    
}

void Scene::renderLightingPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);

    lightingPassShader->begin();

    // 读取GBuffer
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    lightingPassShader->setInt("gPositionWorld", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    lightingPassShader->setInt("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    lightingPassShader->setInt("gAlbedoSpec", 2);

    // 绑定SSAO模糊结果到纹理单元3，并设置uniform
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer); 
    lightingPassShader->setInt("ssao", 3);

    lightingPassShader->setVector3("lightPos", -lightDir * 10.0f);
    lightingPassShader->setVector3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    lightingPassShader->setVector3("viewPos", camera->mPosition);

    // 设置光照参数
    DirectionalLight dirLight {
        lightDir,
        glm::vec3(0.2f, 0.2f, 0.2f),
        glm::vec3(0.4f, 0.4f, 0.2f),
    };
    dirLight.apply(lightingPassShader);

    PointLight pointLight {
        // glm::vec3(6 * cos(time), 0.0f, 6 * sin(timde)),
        glm::vec3(-5.0f, 4.0f, -2.0f),
        glm::vec3(0.7f, 0.7f, 0.7f),
        glm::vec3(2.0f, 2.0f, 2.0f),
    };
    pointLight.apply(lightingPassShader);

    lightingPassShader->setMatrix4x4("lightSpaceMatrix", lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE0 + TEX_DEPTHMAP);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    lightingPassShader->setInt("shadowMap", TEX_DEPTHMAP);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    lightingPassShader->end();

}

void Scene::renderParticlePass() {
    // 在 screenFBO 上绘制粒子，这样它们会进入 screenColorTex，随后参与亮度提取与 Bloom。
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
    // 不改视口（沿用上游设定）

    glEnable(GL_BLEND);
    // 粒子作为发光元素：使用加色混合，使其更容易进入亮度提取阈值
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // 使用从 GBuffer blit 过来的深度，在 screenFBO 中做深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // 透明前向：不开深度写，但要深度测试
    glDepthMask(GL_FALSE);

    // ✅ 新：行向量（世界空间 camera 基）
    const glm::vec3 camRight = camera->getRight();
    const glm::vec3 camUp    = camera->getUp();

    particleShader->begin();
    particleSystem->render(particleShader, camera->getProjectionMatrix() * camera->getViewMatrix(), camRight, camUp);
    particleShader->end();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    // 保持深度测试开启，后续通道会按需修改
}

void Scene::renderPiecesPass() {
    // 碎片行星带：把图像切成格子（两片三角/格），但将每个三角随机分布到圆环上并公转，同时保留每片独立的随机扰动
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO); // forward pass target

    glEnable(GL_DEPTH_TEST);            // depth-test against the blitted depth
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);              // don't write depth for transparent

    glEnable(GL_BLEND);                 // enable alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // 懒构建：带独立 UV 的小矩形几何缓存，附带每片三角的随机种子
    struct TileGeom { Geometry* geo; float a0, a1, b0, b1; };
    static std::vector<TileGeom> s_tiles;
    static bool s_built = false;

    // 切割网格（可调）
    const int COLS = 8;            // 列数
    const int ROWS = 6;            // 行数

    if (!s_built) {
        s_tiles.reserve(COLS * ROWS);

        std::mt19937 rng(1337u);
        std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

        // 基础局部顶点（单位方片，真正大小靠 model 缩放）
        const std::vector<float> baseVerts = {
            -0.5f,-0.5f,0.0f,
             0.5f,-0.5f,0.0f,
             0.5f, 0.5f,0.0f,
            -0.5f, 0.5f,0.0f
        };

        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                float u0 = (float)c / (float)COLS;
                float v0 = (float)r / (float)ROWS;
                float u1 = (float)(c + 1) / (float)COLS;
                float v1 = (float)(r + 1) / (float)ROWS;

                // 如果你知道贴图尺寸，可以把 padU = 0.5/width, padV = 0.5/height，减少出血
                float padU = 0.0f, padV = 0.0f;
                u0 += padU; v0 += padV; u1 -= padU; v1 -= padV;

                std::vector<float> uvs = { u0,v0,  u1,v0,  u1,v1,  u0,v1 };
                Geometry* g = Geometry::createRectangle(baseVerts, uvs);
                s_tiles.push_back({ g, dist01(rng), dist01(rng), dist01(rng), dist01(rng) });
            }
        }
        s_built = true;
    }

    // 将碎片随机分布到一个圆环上：行星带
    const float innerR = 7.0f;    // 内半径（可调）
    const float outerR = 9.0f;    // 外半径（可调）
    const float angularSpeed = 0.05f; // 公转角速度（弧度/秒，可调）
    const float ringTiltDeg = 7.5f; // s圆环倾角（度）
    const float splitGap = 0.015f;   // 两个三角的分离距离（局部坐标小偏移，可调）
    const float triScale = 0.6f;     // 三角整体缩放（相对原 tile 尺寸）
    const float jitterYMul = 2.8f;   // Y 轴扰动增强系数（>1 放大，=1 保持原幅度）
    const float baseAlpha   = 0.45f; // 基础透明度（0~1）
    const float alphaAmp    = 0.25f; // 透明度抖动振幅（建议 0.1~0.35）
    const float alphaFloor  = 0.05f; // 最小透明度地板，避免过暗

    // 倾斜矩阵（绕 X 轴）
    glm::mat4 ringTiltM = glm::rotate(glm::mat4(1.0f), glm::radians(ringTiltDeg), glm::vec3(1,0,0));

    geometryPassShader->begin();
    geometryPassShader->setMatrix4x4("viewMatrix", camera->getViewMatrix());
    geometryPassShader->setMatrix4x4("projectionMatrix", camera->getProjectionMatrix());
    geometryPassShader->setBool("useNormalMap", false);
    geometryPassShader->setBool("isGlowing", false);
    geometryPassShader->setInt("texture_diffuse1", TEX_SANA);
    texture_sana->bind(TEX_SANA);

    float t = glfwGetTime();
    int idx = 0;
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            glBindVertexArray(s_tiles[idx].geo->getVao());

            // --- 以圆环参数计算每个 tile 的两个独立三角的位置 ---
            // 基于 tile 索引生成两个不同的随机角/半径（构造时已写入 s_tiles）
            const TileGeom& tg = s_tiles[idx];

            // Triangle A 的极坐标（随时间公转）
            float thetaA = tg.a0 * glm::two_pi<float>() + angularSpeed * t;
            float radiusA = glm::mix(innerR, outerR, tg.a1);

            // Triangle B 的极坐标（随时间公转，使用不同的随机种子）
            float thetaB = tg.b0 * glm::two_pi<float>() - angularSpeed * 0.85f * t; // 反向或略不同速度
            float radiusB = glm::mix(innerR, outerR, tg.b1);

            // 局部抖动/自转（保持独立碎片随机扰动）
            float jitterXA = 0.06f * sinf(t * 1.37f + (r * 13 + c) * 0.71f + 0.5f);
            float jitterYA = 0.06f * cosf(t * 1.11f + (r * 17 + c) * 0.63f + 1.3f);
            jitterYA *= jitterYMul;
            float spinA    = 0.8f  * sinf(t * 0.97f + (r * 7  + c) * 0.47f + 0.2f);

            float jitterXB = 0.06f * sinf(t * 1.53f + (r * 19 + c) * 0.59f + 2.1f);
            float jitterYB = 0.06f * cosf(t * 1.27f + (r * 11 + c) * 0.67f + 0.9f);
            jitterYB *= jitterYMul;
            float spinB    = 0.8f  * cosf(t * 1.03f + (r * 5  + c) * 0.51f + 0.6f);

            // 将极坐标转换为圆环上的世界位置（先在 XZ 平面上，后施加倾角）
            auto ringPos = [&](float radius, float theta){
                glm::vec4 p(radius * cosf(theta), 0.0f, radius * sinf(theta), 1.0f);
                p = ringTiltM * p; // 施加倾角
                return glm::vec3(p);
            };

            glm::vec3 posA = ringPos(radiusA, thetaA);
            glm::vec3 posB = ringPos(radiusB, thetaB);

            // 让三角片朝向摄像机（面朝相机的 billboarding，仅绕 Y 轴）
            auto faceCameraYaw = [&](const glm::vec3& pos){
                glm::vec3 toCam = glm::normalize(camera->mPosition - pos);
                float yaw = atan2f(toCam.x, toCam.z);
                return yaw;
            };

            float yawA = faceCameraYaw(posA);
            float yawB = faceCameraYaw(posB);

            // Triangle A 变换：平移到环上 + 朝向相机 + 自身轻微旋转 + 轻微分离
            {
                glm::mat4 M(1.0f);
                M = glm::translate(M, posA + glm::vec3(jitterXA, jitterYA * 0.5f, 0.0f));
                M = glm::rotate(M, yawA, glm::vec3(0,1,0));
                M = glm::rotate(M, spinA, glm::vec3(0,0,1)); // 局部自转（在片面内）
                M = glm::translate(M, glm::vec3(+splitGap, -splitGap, 0.0f)); // 与 B 片拉开一点距离
                M = glm::scale(M, glm::vec3(triScale, triScale, 1.0f));

                geometryPassShader->setMatrix4x4("model", M);
                float alphaA = baseAlpha + alphaAmp * sinf(t * 1.13f + (r * 19 + c) * 0.37f);
                geometryPassShader->setFloat("u_Alpha", glm::clamp(alphaFloor + alphaA, 0.0f, 1.0f));
                geometryPassShader->setBool("u_ForwardColor", true);
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(0));
            }

            // Triangle B 变换：平移到环上 + 朝向相机 + 自身轻微旋转 + 反向分离
            {
                glm::mat4 M(1.0f);
                M = glm::translate(M, posB + glm::vec3(jitterXB, jitterYB * 0.5f, 0.0f));
                M = glm::rotate(M, yawB, glm::vec3(0,1,0));
                M = glm::rotate(M, spinB, glm::vec3(0,0,1));
                M = glm::translate(M, glm::vec3(-splitGap, +splitGap, 0.0f));
                M = glm::scale(M, glm::vec3(triScale, triScale, 1.0f));

                geometryPassShader->setMatrix4x4("model", M);
                float alphaB = baseAlpha + alphaAmp * cosf(t * 1.07f + (r * 23 + c) * 0.41f + 0.7f);
                geometryPassShader->setFloat("u_Alpha", glm::clamp(alphaFloor + alphaB, 0.0f, 1.0f));
                geometryPassShader->setBool("u_ForwardColor", true);
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * sizeof(unsigned int)));
            }

            // --- Extra clones to double triangle count (A' and B') ---
            // Small angular and radial offsets so the clones don't overlap perfectly
            const float dTheta = 0.18f;                 // angular offset (radians)
            const float dRad   = 0.06f;                 // radial percentage offset
            const float triScale2 = triScale * 0.95f;   // tiny size tweak for visual richness

            // Clone of Triangle A (A')
            {
                float thetaA2  = thetaA + dTheta;
                float radiusA2 = radiusA * (1.0f + dRad * sinf(t * 0.73f + (r * 3 + c) * 0.41f));
                glm::vec3 posA2 = ringPos(radiusA2, thetaA2);

                float jitterXA2 = 0.06f * sinf(t * 1.81f + (r * 29 + c) * 0.53f + 0.7f);
                float jitterYA2 = 0.06f * cosf(t * 1.37f + (r * 23 + c) * 0.57f + 1.1f);
                jitterYA2 *= jitterYMul;
                float spinA2    = 0.8f  * sinf(t * 1.21f + (r * 9  + c) * 0.39f + 0.4f);

                glm::mat4 M(1.0f);
                M = glm::translate(M, posA2 + glm::vec3(jitterXA2, jitterYA2 * 0.5f, 0.0f));
                M = glm::rotate(M, yawA, glm::vec3(0,1,0));
                M = glm::rotate(M, spinA2, glm::vec3(0,0,1));
                M = glm::translate(M, glm::vec3(+splitGap * 0.6f, -splitGap * 0.6f, 0.0f));
                M = glm::scale(M, glm::vec3(triScale2, triScale2, 1.0f));

                geometryPassShader->setMatrix4x4("model", M);
                float alphaA2 = baseAlpha + alphaAmp * sinf(t * 0.91f + (r * 31 + c) * 0.33f + 1.2f);
                geometryPassShader->setFloat("u_Alpha", glm::clamp(alphaFloor + alphaA2, 0.0f, 1.0f));
                geometryPassShader->setBool("u_ForwardColor", true);
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(0)); // same indices as A
            }

            // Clone of Triangle B (B')
            {
                float thetaB2  = thetaB - dTheta;
                float radiusB2 = radiusB * (1.0f + dRad * cosf(t * 0.69f + (r * 17 + c) * 0.37f));
                glm::vec3 posB2 = ringPos(radiusB2, thetaB2);

                float jitterXB2 = 0.06f * sinf(t * 1.67f + (r * 31 + c) * 0.61f + 1.9f);
                float jitterYB2 = 0.06f * cosf(t * 1.43f + (r * 27 + c) * 0.49f + 0.8f);
                jitterYB2 *= jitterYMul;
                float spinB2    = 0.8f  * cosf(t * 1.09f + (r * 15 + c) * 0.45f + 0.3f);

                glm::mat4 M(1.0f);
                M = glm::translate(M, posB2 + glm::vec3(jitterXB2, jitterYB2 * 0.5f, 0.0f));
                M = glm::rotate(M, yawB, glm::vec3(0,1,0));
                M = glm::rotate(M, spinB2, glm::vec3(0,0,1));
                M = glm::translate(M, glm::vec3(-splitGap * 0.6f, +splitGap * 0.6f, 0.0f));
                M = glm::scale(M, glm::vec3(triScale2, triScale2, 1.0f));

                geometryPassShader->setMatrix4x4("model", M);
                float alphaB2 = baseAlpha + alphaAmp * cosf(t * 1.19f + (r * 17 + c) * 0.29f + 2.4f);
                geometryPassShader->setFloat("u_Alpha", glm::clamp(alphaFloor + alphaB2, 0.0f, 1.0f));
                geometryPassShader->setBool("u_ForwardColor", true);
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * sizeof(unsigned int))); // same indices as B
            }
            ++idx;
        }
    }

    geometryPassShader->end();
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);
}

void Scene::renderBrightExtractPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, bloomBrightFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ✅ 包含 GL_DEPTH_BUFFER_BIT

    brightExtractShader->begin();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenColorTex);  // 来自 screenFBO 的最终主场景颜色结果
    brightExtractShader->setInt("scene", 0);
    brightExtractShader->setFloat("threshold", 1.0f);  // 可调节

    // Bind bloom mask texture before drawing
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBloomMask);
    brightExtractShader->setInt("bloomMask", 1);


    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    brightExtractShader->end();

    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::renderBloomBlurPass() {
    bool horizontal = true;
    bool first_iteration = true;
    int blurPasses = 10;

    blurShader->begin();

    for (int i = 0; i < blurPasses; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, bloomPingpongFBO[horizontal]);

        blurShader->setInt("horizontal", horizontal ? 1 : 0);

        glActiveTexture(GL_TEXTURE0);
        if (first_iteration) {
            glBindTexture(GL_TEXTURE_2D, bloomBrightTex);
            first_iteration = false;
        } else {
            glBindTexture(GL_TEXTURE_2D, bloomPingpongTex[!horizontal]);
        }
        blurShader->setInt("image", 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        horizontal = !horizontal;
    }

    blurShader->end();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::renderBloomCompositePass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // 输出到默认屏幕帧缓冲
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    finalCompositeShader->begin();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenColorTex);  // 主图
    finalCompositeShader->setInt("scene", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomPingpongTex[1]);  // 模糊后的 bloom 亮度图
    finalCompositeShader->setInt("bloomBlur", 1);

    finalCompositeShader->setFloat("exposure", 1.0f);  // 曝光值可调节

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    finalCompositeShader->end();
}

void Scene::renderText() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // ✅ 确保写入默认FBO
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 设置正交投影矩阵并传递给 textShader
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(gl_app.getWidth()), 0.0f, static_cast<float>(gl_app.getHeight()));
    textShader->begin();
    textShader->setMatrix4x4("projection", projection);
    // textShader->setVector3("textColor", glm::vec3(1.0, 1.0, 1.0));         // 白字
    textShader->setVector3("outlineColor", glm::vec3(0.0, 0.0, 0.0));      // 黑边
    textShader->setVector3("shadowColor", glm::vec3(0.1, 0.1, 0.1));       // 阴影深灰
    textShader->setVector2("pixelSize", glm::vec2(1.0f / gl_app.getWidth(), 1.0f / gl_app.getHeight()));
    textShader->end();

    glm::vec3 color(1.0f, 1.0f, 1.0f);  // 白色
    textRenderer->renderText("Hello, OpenGL!", 25.0f, gl_app.getHeight() - 50.0f, 2.0f, color, textShader);

    glDisable(GL_BLEND);  // 可选
    glEnable(GL_DEPTH_TEST);
}

// 渲染发光三角形线条
void Scene::renderGlowObjects() {
    glEnable(GL_DEPTH_TEST);     // Enable depth testing
    glDepthMask(GL_TRUE);        // Allow writing to depth buffer

    // 动画控制：严格在第 9 秒窗口 [8,9) 绘制（9s 为循环周期）
    float time = glfwGetTime();
    const float cycle = 8.0f;                 // 9 秒一个周期
    float inCycle = fmod(time, cycle);        // [0,9)
    const float pulseLen   = 1.0f;            // 第 9 秒整秒都绘制
    const float pulseStart = 7.0f;            // 第 8~9 秒作为绘制窗口
    const float pulseEnd   = pulseStart + pulseLen; // 9.0
    bool pulse = (inCycle >= pulseStart && inCycle < pulseEnd);

    glowShader->begin();

    // 计算脉冲归一化时间（仅用于厚度），不再缩放三角形
    float tNorm = 0.0f;      // 0..1 within [8,9)
    if (pulse) tNorm = (inCycle - pulseStart) / pulseLen;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.51f));
    glowShader->setMatrix4x4("u_Model", model);
    glowShader->setMatrix4x4("u_ViewProjection", camera->getProjectionMatrix() * camera->getViewMatrix());
    glowShader->setVector3("u_Color", glm::vec3(1.0f, 0.8f, 0.3f)); // 明亮橙色
    // u_Thickness will be set below

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blend

    glBindVertexArray(triangleGeometry->getVao());

    if (pulse) {
        // 将 1s 切成 3 段：每段 ~0.333s 依次点亮一条边
        float segF = tNorm * 3.0f;                 // [0,3)
        int edgeIndex = glm::clamp(int(floorf(segF)), 0, 2); // 当前进行到的边（0,1,2）
        float segT = segF - floorf(segF);          // 当前边内部 0..1 进度

        // 厚度设置：已出现的边=恒定粗；当前边=包络 0→1→0；未出现的边=不画
        const float pulseThickness = 0.005f;         // 调大以“增宽光带”
        float envelope = sin(segT * 3.14159265f);  // 当前边的淡入淡出

        // 依次绘制 [0 .. edgeIndex] 这些边，保证已出现的边保持可见
        for (int i = 0; i <= edgeIndex; ++i) {
            float thick = (i < edgeIndex) ? pulseThickness : (pulseThickness * envelope);
            glowShader->setFloat("u_Thickness", thick);

            // 给 shader 一个有效的 u_Progress（几何着色器按 0..1 范围使用它）
            glowShader->setFloat("u_Progress", 1.0f);

            // 仅绘制第 i 条边（EBO 顺序：0,1, 1,2, 2,0，每条边 2 索引）
            GLsizeiptr edgeOffsetBytes = static_cast<GLsizeiptr>(i * 2 * sizeof(unsigned int));
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, reinterpret_cast<void*>(edgeOffsetBytes));
        }
    } else {
        // 非脉冲期不绘制（可改为常亮细线：设置一个很小的 thickness 并绘制一次）
        glowShader->setFloat("u_Thickness", 0.0f);
    }

    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glowShader->end();
}

void Scene::renderSkybox() {
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glDepthFunc(GL_LEQUAL);   // 只需要控制 depth 测试顺序
    glDepthMask(GL_FALSE);    // 禁止写入 depth，避免污染已有 depth
    skybox->draw(glm::mat4(glm::mat3(camera->getViewMatrix())), camera->getProjectionMatrix());
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

// 测试使用
void Scene::renderTest() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBloomMask);  // ssaoColorBuffer
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);  // ✅ 新增这行

    screenShader->begin();
    screenShader->setInt("screenTexture", 0);
    screenShader->setInt("debugMode", 1);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    screenShader->end();
}