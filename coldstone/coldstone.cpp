#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <string>
#include <iostream>
#include <format>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderGraph.h"
#include "GLCommandBuffer.h"
#include "GLResourcePool.h"
#include "GLFramebuffer.h"
#include "GLPipeline.h"
#include "Input.h"
#include "Camera.h"
#include "Scene.h"
#include "Shaders.h"
#include "TextRenderer.h"

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float radius;
};

int main(void) {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Coldstone Engine", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    Input::Initialize(window);
    ilInit(); iluInit();
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;

    Scene::Initialize();
    Camera camera(glm::vec3(0.0f, 6.0f, 12.0f));
    GLResourcePool resourcePool;
    GLCommandBuffer cmdBuffer;
    DigitRenderer digitRenderer;
    digitRenderer.Init();

    Shader shadowShader(SHADOW_VS, SHADOW_FS, true);
    Shader gBufferShader(GBUFFER_VS, GBUFFER_FS, true);
    Shader lightingShader(LIGHTING_VS, LIGHTING_FS, true);
    Shader forwardShader(FORWARD_VS, FORWARD_FS, true);
    Shader blurShader(BLUR_VS, BLUR_FS, true);
    Shader compositeShader(COMPOSITE_VS, COMPOSITE_FS, true);

    GLPipeline shadowPipeline;
    shadowPipeline.shader = &shadowShader;
    shadowPipeline.depthTest = true;
    shadowPipeline.cullFace = true;
    shadowPipeline.cullMode = GL_FRONT;

    GLPipeline gBufferPipeline;
    gBufferPipeline.shader = &gBufferShader;
    gBufferPipeline.depthTest = true;
    gBufferPipeline.depthWrite = true;

    GLPipeline lightingPipeline;
    lightingPipeline.shader = &lightingShader;
    lightingPipeline.depthTest = false;
    lightingPipeline.blend = false;

    GLPipeline forwardPipeline;
    forwardPipeline.shader = &forwardShader;
    forwardPipeline.depthTest = true;
    forwardPipeline.depthWrite = false;
    forwardPipeline.blend = true;
    forwardPipeline.blendSrc = GL_SRC_ALPHA;
    forwardPipeline.blendDst = GL_ONE_MINUS_SRC_ALPHA;
    forwardPipeline.cullFace = true;

    GLPipeline blurPipeline;
    blurPipeline.shader = &blurShader;
    blurPipeline.depthTest = false;
    blurPipeline.blend = false;

    GLPipeline compositePipeline;
    compositePipeline.shader = &compositeShader;
    compositePipeline.depthTest = false;
    compositePipeline.blend = false;

    GLFramebuffer gBufferFBO(1280, 720);
    GLFramebuffer lightingFBO(1280, 720);
    GLFramebuffer pingPongFBO[2] = { GLFramebuffer(1280, 720), GLFramebuffer(1280, 720) };

    const int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    GLFramebuffer shadowFBO(SHADOW_WIDTH, SHADOW_HEIGHT);

    struct ShadowData { TextureHandle depthMap; };
    struct GBufferData { TextureHandle albedo, normal, material, depth; };
    struct LightingData { TextureHandle sceneColor, brightColor, depthBuffer; };
    struct ForwardData { TextureHandle sceneColorInOut; TextureHandle depthInput; };
    struct BlurData { TextureHandle inputBright; TextureHandle blur1, blur2; TextureHandle finalBloom; };
    struct CompositeData { TextureHandle inputScene; TextureHandle inputBloom; };

    TextureHandle hShadowDepth;
    TextureHandle hAlbedo, hNormal, hMaterial, hDepth;
    TextureHandle hSceneColor, hBrightColor;
    TextureHandle hFinalBloom;

    double lastTime = glfwGetTime();
    int nbFrames = 0;
    std::string fpsString = "0";

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) {
            fpsString = std::format("{}", nbFrames);
            nbFrames = 0;
            lastTime += 1.0;
        }

        Input::Update();
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        camera.Update(0.016f, width, height);

        gBufferFBO.Resize(width, height);
        lightingFBO.Resize(width, height);
        pingPongFBO[0].Resize(width, height);
        pingPongFBO[1].Resize(width, height);

        std::vector<PointLight> sceneLights;
        for (const auto& obj : Scene::objects) {
            float emissionMax = std::max(obj.material.emission.r, std::max(obj.material.emission.g, obj.material.emission.b));
            if (emissionMax > 0.1f) {
                sceneLights.push_back({ obj.position, obj.material.emission, 10.0f });
            }
        }

        glm::vec3 sunPos = glm::vec3(-10.0f, 20.0f, -10.0f);
        glm::vec3 sunDir = glm::normalize(sunPos - glm::vec3(0, 0, 0));
        glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 1.0f, 100.0f);
        glm::mat4 lightView = glm::lookAt(sunPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        RenderGraph renderGraph(&resourcePool);

        renderGraph.addPass<ShadowData>("ShadowPass",
            [&](RGBuilder& builder, ShadowData& data) {
                TextureDesc desc = { (uint32_t)SHADOW_WIDTH, (uint32_t)SHADOW_HEIGHT, TextureFormat::DEPTH24STENCIL8, false };
                data.depthMap = builder.create(desc, "ShadowMap");
                hShadowDepth = data.depthMap;
            },
            [&](ShadowData& data, RGRegistry& registry, ICommandBuffer* cmd) {
                GLCommandBuffer* glCmd = static_cast<GLCommandBuffer*>(cmd);
                glCmd->setRegistry(&registry);
                shadowFBO.AttachTexture((GLTexture*)registry.getTexture(data.depthMap), GL_DEPTH_ATTACHMENT);
                shadowFBO.Finalize();
                glCmd->setViewport(SHADOW_WIDTH, SHADOW_HEIGHT);
                glClear(GL_DEPTH_BUFFER_BIT);
                shadowPipeline.Bind();
                glCmd->setUniformMat4(shadowPipeline.shader, "lightSpaceMatrix", lightSpaceMatrix);

                for (auto& obj : Scene::objects) {
                    if (obj.material.transparency > 0.0f) continue;
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, obj.position);
                    model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0, 1, 0));
                    model = glm::scale(model, obj.scale);
                    glCmd->setUniformMat4(shadowPipeline.shader, "model", model);
                    glCmd->bindVertexArray(obj.mesh->VAO);
                    glCmd->drawIndexed((unsigned int)obj.mesh->indices.size());
                }
            }
        );

        renderGraph.addPass<GBufferData>("GBuffer",
            [&](RGBuilder& builder, GBufferData& data) {
                TextureDesc desc = { (uint32_t)width, (uint32_t)height, TextureFormat::RGBA16F, false };
                data.albedo = builder.create(desc, "Albedo");
                data.normal = builder.create(desc, "Normal");
                data.material = builder.create(desc, "Material");
                TextureDesc dDesc = { (uint32_t)width, (uint32_t)height, TextureFormat::DEPTH24STENCIL8, false };
                data.depth = builder.create(dDesc, "Depth");

                hAlbedo = data.albedo;
                hNormal = data.normal;
                hMaterial = data.material;
                hDepth = data.depth;
            },
            [&](GBufferData& data, RGRegistry& registry, ICommandBuffer* cmd) {
                GLCommandBuffer* glCmd = static_cast<GLCommandBuffer*>(cmd);
                glCmd->setRegistry(&registry);
                gBufferFBO.AttachTexture((GLTexture*)registry.getTexture(data.albedo), GL_COLOR_ATTACHMENT0);
                gBufferFBO.AttachTexture((GLTexture*)registry.getTexture(data.normal), GL_COLOR_ATTACHMENT1);
                gBufferFBO.AttachTexture((GLTexture*)registry.getTexture(data.material), GL_COLOR_ATTACHMENT2);
                gBufferFBO.AttachTexture((GLTexture*)registry.getTexture(data.depth), GL_DEPTH_STENCIL_ATTACHMENT);
                gBufferFBO.Finalize();

                glCmd->setViewport(width, height);
                glCmd->clear(0.0f, 0.0f, 0.0f, 1.0f);
                gBufferPipeline.Bind();
                glCmd->setUniformMat4(gBufferPipeline.shader, "view", camera.GetViewMatrix());
                glCmd->setUniformMat4(gBufferPipeline.shader, "projection", camera.GetProjectionMatrix(width, height));

                for (auto& obj : Scene::objects) {
                    if (obj.material.transparency > 0.0f) continue;

                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, obj.position);
                    model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0, 1, 0));
                    model = glm::scale(model, obj.scale);
                    glCmd->setUniformMat4(gBufferPipeline.shader, "model", model);
                    glCmd->setUniformVec3(gBufferPipeline.shader, "material.albedo", obj.material.albedo);
                    glCmd->setUniformFloat(gBufferPipeline.shader, "material.metallic", obj.material.metallic);
                    glCmd->setUniformFloat(gBufferPipeline.shader, "material.roughness", obj.material.roughness);
                    glCmd->setUniformVec3(gBufferPipeline.shader, "material.emission", obj.material.emission);
                    glCmd->bindVertexArray(obj.mesh->VAO);
                    glCmd->drawIndexed((unsigned int)obj.mesh->indices.size());
                }
            }
        );

        renderGraph.addPass<LightingData>("Lighting",
            [&](RGBuilder& builder, LightingData& data) {
                builder.read(hShadowDepth);
                builder.read(hAlbedo);
                builder.read(hNormal);
                builder.read(hMaterial);
                builder.read(hDepth);

                TextureDesc desc = { (uint32_t)width, (uint32_t)height, TextureFormat::RGBA16F, false };
                data.sceneColor = builder.create(desc, "SceneColor");
                data.brightColor = builder.create(desc, "BrightColor");

                // Create a depth buffer for the lighting FBO so we can blit to it later
                TextureDesc dDesc = { (uint32_t)width, (uint32_t)height, TextureFormat::DEPTH24STENCIL8, false };
                data.depthBuffer = builder.create(dDesc, "LightingDepth");

                hSceneColor = data.sceneColor;
                hBrightColor = data.brightColor;
            },
            [&](LightingData& data, RGRegistry& registry, ICommandBuffer* cmd) {
                GLCommandBuffer* glCmd = static_cast<GLCommandBuffer*>(cmd);
                glCmd->setRegistry(&registry);

                lightingFBO.AttachTexture((GLTexture*)registry.getTexture(data.sceneColor), GL_COLOR_ATTACHMENT0);
                lightingFBO.AttachTexture((GLTexture*)registry.getTexture(data.brightColor), GL_COLOR_ATTACHMENT1);
                // Attach the depth buffer to allow blitting later
                lightingFBO.AttachTexture((GLTexture*)registry.getTexture(data.depthBuffer), GL_DEPTH_STENCIL_ATTACHMENT);
                lightingFBO.Finalize();

                glCmd->setViewport(width, height);
                glCmd->clear(0.0f, 0.0f, 0.0f, 1.0f);

                lightingPipeline.Bind();

                glCmd->bindTexture(0, hShadowDepth);
                glCmd->setUniformInt(lightingPipeline.shader, "shadowMap", 0);

                glCmd->bindTexture(1, hAlbedo);
                glCmd->setUniformInt(lightingPipeline.shader, "gAlbedo", 1);

                glCmd->bindTexture(2, hNormal);
                glCmd->setUniformInt(lightingPipeline.shader, "gNormal", 2);

                glCmd->bindTexture(3, hMaterial);
                glCmd->setUniformInt(lightingPipeline.shader, "gMaterial", 3);

                glCmd->bindTexture(4, hDepth);
                glCmd->setUniformInt(lightingPipeline.shader, "gDepth", 4);

                glCmd->setUniformVec3(lightingPipeline.shader, "viewPos", camera.GetPosition());
                glCmd->setUniformMat4(lightingPipeline.shader, "inverseView", glm::inverse(camera.GetViewMatrix()));
                glCmd->setUniformMat4(lightingPipeline.shader, "inverseProj", glm::inverse(camera.GetProjectionMatrix(width, height)));
                glCmd->setUniformVec3(lightingPipeline.shader, "sunDir", sunDir);
                glCmd->setUniformMat4(lightingPipeline.shader, "lightSpaceMatrix", lightSpaceMatrix);

                glCmd->setUniformInt(lightingPipeline.shader, "numPointLights", (int)sceneLights.size());
                for (int i = 0; i < sceneLights.size() && i < 16; ++i) {
                    std::string base = "pointLights[" + std::to_string(i) + "]";
                    glCmd->setUniformVec3(lightingPipeline.shader, base + ".position", sceneLights[i].position);
                    glCmd->setUniformVec3(lightingPipeline.shader, base + ".color", sceneLights[i].color);
                }

                glCmd->bindVertexArray(Scene::quadMesh.VAO);
                glCmd->drawIndexed(6);
            }
        );

        renderGraph.addPass<ForwardData>("Forward",
            [&](RGBuilder& builder, ForwardData& data) {
                data.depthInput = builder.read(hDepth);
                data.sceneColorInOut = builder.read(hSceneColor);
            },
            [&](ForwardData& data, RGRegistry& registry, ICommandBuffer* cmd) {
                GLCommandBuffer* glCmd = static_cast<GLCommandBuffer*>(cmd);

                lightingFBO.Bind();
                // Copy Depth from GBuffer to Lighting FBO so transparent objects depth test correctly
                glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferFBO.GetID());
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightingFBO.GetID());
                glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
                lightingFBO.Bind();

                forwardPipeline.Bind();
                glCmd->setUniformMat4(forwardPipeline.shader, "view", camera.GetViewMatrix());
                glCmd->setUniformMat4(forwardPipeline.shader, "projection", camera.GetProjectionMatrix(width, height));
                glCmd->setUniformVec3(forwardPipeline.shader, "viewPos", camera.GetPosition());
                glCmd->setUniformVec3(forwardPipeline.shader, "sunDir", sunDir);

                for (auto& obj : Scene::objects) {
                    if (obj.material.transparency > 0.0f) {
                        glm::mat4 model = glm::mat4(1.0f);
                        model = glm::translate(model, obj.position);
                        model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0, 1, 0));
                        model = glm::scale(model, obj.scale);
                        glCmd->setUniformMat4(forwardPipeline.shader, "model", model);
                        glCmd->setUniformVec3(forwardPipeline.shader, "color", obj.material.albedo);
                        glCmd->setUniformFloat(forwardPipeline.shader, "alpha", 1.0f - obj.material.transparency);
                        glCmd->setUniformFloat(forwardPipeline.shader, "roughness", obj.material.roughness);

                        glCmd->bindVertexArray(obj.mesh->VAO);

                        glCullFace(GL_FRONT);
                        glCmd->drawIndexed((unsigned int)obj.mesh->indices.size());
                        glCullFace(GL_BACK);
                        glCmd->drawIndexed((unsigned int)obj.mesh->indices.size());
                    }
                }
            }
        );

        renderGraph.addPass<BlurData>("Blur",
            [&](RGBuilder& builder, BlurData& data) {
                data.inputBright = builder.read(hBrightColor);

                TextureDesc desc = { (uint32_t)width, (uint32_t)height, TextureFormat::RGBA16F, false };
                data.blur1 = builder.create(desc, "Blur1");
                data.blur2 = builder.create(desc, "Blur2");
            },
            [&](BlurData& data, RGRegistry& registry, ICommandBuffer* cmd) {
                GLCommandBuffer* glCmd = static_cast<GLCommandBuffer*>(cmd);
                glCmd->setRegistry(&registry);

                pingPongFBO[0].AttachTexture((GLTexture*)registry.getTexture(data.blur1), GL_COLOR_ATTACHMENT0);
                pingPongFBO[0].Finalize();
                pingPongFBO[1].AttachTexture((GLTexture*)registry.getTexture(data.blur2), GL_COLOR_ATTACHMENT0);
                pingPongFBO[1].Finalize();

                bool horizontal = true, first_iteration = true;
                int amount = 6;
                blurPipeline.Bind();

                for (unsigned int i = 0; i < amount; i++) {
                    pingPongFBO[horizontal].Bind();
                    glCmd->setUniformInt(blurPipeline.shader, "horizontal", horizontal);

                    glActiveTexture(GL_TEXTURE0);
                    if (first_iteration) {
                        glCmd->bindTexture(0, data.inputBright);
                    }
                    else {
                        glCmd->bindTexture(0, horizontal ? data.blur2 : data.blur1);
                    }

                    glCmd->bindVertexArray(Scene::quadMesh.VAO);
                    glCmd->drawIndexed(6);

                    horizontal = !horizontal;
                    if (first_iteration) first_iteration = false;
                }
                hFinalBloom = data.blur1;
            }
        );

        renderGraph.addPass<CompositeData>("Composite",
            [&](RGBuilder& builder, CompositeData& data) {
                data.inputScene = builder.read(hSceneColor);
                data.inputBloom = builder.read(hFinalBloom);
            },
            [&](CompositeData& data, RGRegistry& registry, ICommandBuffer* cmd) {
                GLCommandBuffer* glCmd = static_cast<GLCommandBuffer*>(cmd);
                glCmd->setRegistry(&registry);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glCmd->setViewport(width, height);
                glCmd->clear(0.0f, 0.0f, 0.0f, 1.0f);

                compositePipeline.Bind();

                glCmd->bindTexture(0, data.inputScene);
                glCmd->setUniformInt(compositePipeline.shader, "scene", 0);

                glCmd->bindTexture(1, data.inputBloom);
                glCmd->setUniformInt(compositePipeline.shader, "bloomBlur", 1);

                glCmd->bindVertexArray(Scene::quadMesh.VAO);
                glCmd->drawIndexed(6);

                glDisable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                digitRenderer.RenderString(glCmd, fpsString, 10, height - 40, 1.0f, width, height);
                glDisable(GL_BLEND);
            }
        );

        renderGraph.compile();
        renderGraph.execute(&cmdBuffer);
        renderGraph.reset();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}