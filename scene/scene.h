#pragma once

#include "../camera/perspectiveCamera.h"
#include "../camera/gameCameraControl.h"
#include "../core/shader.h"
#include "../core/geometry.h"
// #include "../core/framebuffer.h"
#include "../core/texture.h"
#include "../resource/light.h"
#include "../particle/particleSystem.h"
#include "../renderer/environment/skybox/skybox.h"
#include "../animation/trajectory.h"
#include "../text/textrenderer.h"

class Scene
{
public:
    void init();
    void update();
    void render();
    void cleanup();

private:
    // prepare设置
    void prepareShaders();
    void prepareGeometry();
    void prepareGBuffer();
    void prepareScreenFBO();
    void prepareFullscreen();  
    void prepareCamera();
    void prepareTexture();
    void prepareSkybox();
    void prepareShadowMap();
    void prepareSSAO();
    void prepareBloom();
    void prepareText();

    // render设置
    void renderGeometryPass();
    void renderLightingPass();
    void renderSkybox();
    void renderShadowMapPass();
    void renderSSAOPass();
    void renderPiecesPass();
    void renderParticlePass();
    void renderGlowObjects();
    void renderBrightExtractPass();
    void renderBloomBlurPass();
    void renderBloomCompositePass();
    void renderText();

    // 测试使用
    void renderTest();

    // Camera
    PerspectiveCamera* camera = nullptr;

    // CameraControl
    GameCameraControl* cameraControl = nullptr;

    // Geometry
    Geometry* geometry = nullptr;
    Geometry* floorGeometry = nullptr;
    Geometry* piecesGeometry = nullptr;
    Geometry* triangleGeometry = nullptr;
    std::vector<Geometry*> rectangleGeometry;
    std::vector<Geometry*> squareGeometry;

    // Texture
    SingleTexture* texture_sana = nullptr;
    SingleTexture* texture_floor = nullptr;
    SingleTexture* normal_water = nullptr;

    // Particle
    ParticleSystem* particleSystem = nullptr;

    // Shaders
    Shader* geometryPassShader = nullptr;
    Shader* lightingPassShader = nullptr;
    Shader* screenShader = nullptr;
    Shader* depthShader = nullptr;
    Shader* ssaoShader = nullptr;
    Shader* ssaoBlurShader = nullptr;
    Shader* particleShader = nullptr;
    Shader* glowShader = nullptr;
    Shader* brightExtractShader = nullptr;
    Shader* blurShader = nullptr;
    Shader* finalCompositeShader = nullptr;
    Shader* textShader = nullptr;

    // 天空盒
    Skybox *skybox = nullptr;   

    // 阴影贴图
    GLuint shadowFBO, shadowMap;
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    glm::mat4 lightSpaceMatrix;

    // GBuffer framebuffer & textures
    GLuint gBuffer = 0;
    GLuint gPosition = 0;
    GLuint gNormal = 0;
    GLuint gPositionView = 0;
    GLuint gAlbedoSpec = 0;
    GLuint gLinearDepth = 0;
    GLuint gDepth;
    GLuint gBloomMask;

    // Fullscreen quad VAO/VBO
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    GLuint screenFBO, screenColorTex, screenDepth;

    // SSAO
    GLuint ssaoFBO = 0;
    GLuint ssaoColorBuffer = 0;
    GLuint noiseTex = 0;
    GLuint ssaoBlurFBO = 0;
    GLuint ssaoBlurColorBuffer = 0;
    std::vector<glm::vec3> ssaoKernel;

    // Bloom
    GLuint bloomBrightFBO = 0;
    GLuint bloomBrightTex = 0;
    GLuint bloomPingpongFBO[2];
    GLuint bloomPingpongTex[2];
    GLuint bloomExtractDepthRBO = 0;

    // Text
    TextRenderer* textRenderer = nullptr;
};