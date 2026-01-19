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
    Camera camera(glm::vec3(0.0f, 8.0f, 15.0f));
    GLResourcePool resourcePool;
    GLCommandBuffer cmdBuffer;
    DigitRenderer digitRenderer;
    digitRenderer.Init();

    Shader shadowShader(SHADOW_VS, SHADOW_FS, true);
    Shader gBufferShader(GBUFFER_VS, GBUFFER_FS, true);
    Shader lightingShader(LIGHTING_VS, LIGHTING_FS, true);
    Shader forwardShader(FORWARD_VS, FORWARD_FS, true);

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

    GLFramebuffer gBufferFBO(1280, 720);
    const int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    GLFramebuffer shadowFBO(SHADOW_WIDTH, SHADOW_HEIGHT);

    struct ShadowData { TextureHandle depthMap; };
    struct GBufferData { TextureHandle albedo, normal, material, depth; };
    struct LightingData { TextureHandle output; };

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
                    if (obj.material.roughness < 0.02f && obj.material.metallic == 0.0f) continue;

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
                    if (obj.material.roughness < 0.02f && obj.material.metallic == 0.0f) continue;

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
                builder.read(TextureHandle{ 0 });
                builder.read(TextureHandle{ 1 });
                builder.read(TextureHandle{ 2 });
                builder.read(TextureHandle{ 3 });
                builder.read(TextureHandle{ 4 });
            },
            [&](LightingData& data, RGRegistry& registry, ICommandBuffer* cmd) {
                GLCommandBuffer* glCmd = static_cast<GLCommandBuffer*>(cmd);
                glCmd->setRegistry(&registry);

                gBufferFBO.Unbind();
                glCmd->setViewport(width, height);
                glCmd->clear(0.0f, 0.0f, 0.0f, 1.0f);

                lightingPipeline.Bind();

                glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, ((GLTexture*)registry.getTexture(TextureHandle{ 0 }))->getGLHandle());
                glCmd->setUniformInt(lightingPipeline.shader, "shadowMap", 0);
                glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, ((GLTexture*)registry.getTexture(TextureHandle{ 1 }))->getGLHandle());
                glCmd->setUniformInt(lightingPipeline.shader, "gAlbedo", 1);
                glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, ((GLTexture*)registry.getTexture(TextureHandle{ 2 }))->getGLHandle());
                glCmd->setUniformInt(lightingPipeline.shader, "gNormal", 2);
                glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, ((GLTexture*)registry.getTexture(TextureHandle{ 3 }))->getGLHandle());
                glCmd->setUniformInt(lightingPipeline.shader, "gMaterial", 3);
                glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, ((GLTexture*)registry.getTexture(TextureHandle{ 4 }))->getGLHandle());
                glCmd->setUniformInt(lightingPipeline.shader, "gDepth", 4);

                glCmd->setUniformVec3(lightingPipeline.shader, "viewPos", camera.GetPosition());
                glCmd->setUniformMat4(lightingPipeline.shader, "inverseView", glm::inverse(camera.GetViewMatrix()));
                glCmd->setUniformMat4(lightingPipeline.shader, "inverseProj", glm::inverse(camera.GetProjectionMatrix(width, height)));
                glCmd->setUniformVec3(lightingPipeline.shader, "sunDir", sunDir);
                glCmd->setUniformMat4(lightingPipeline.shader, "lightSpaceMatrix", lightSpaceMatrix);

                glCmd->bindVertexArray(Scene::quadMesh.VAO);
                glCmd->drawIndexed(6);
            }
        );

        renderGraph.addPass<LightingData>("Forward",
            [&](RGBuilder& builder, LightingData& data) {},
            [&](LightingData& data, RGRegistry& registry, ICommandBuffer* cmd) {
                GLCommandBuffer* glCmd = static_cast<GLCommandBuffer*>(cmd);

                glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferFBO.GetID());
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                forwardPipeline.Bind();
                glCmd->setUniformMat4(forwardPipeline.shader, "view", camera.GetViewMatrix());
                glCmd->setUniformMat4(forwardPipeline.shader, "projection", camera.GetProjectionMatrix(width, height));
                glCmd->setUniformVec3(forwardPipeline.shader, "viewPos", camera.GetPosition());
                glCmd->setUniformVec3(forwardPipeline.shader, "sunDir", sunDir);

                for (auto& obj : Scene::objects) {
                    if (obj.material.roughness < 0.02f && obj.material.metallic == 0.0f) {
                        glm::mat4 model = glm::mat4(1.0f);
                        model = glm::translate(model, obj.position);
                        model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0, 1, 0));
                        model = glm::scale(model, obj.scale);
                        glCmd->setUniformMat4(forwardPipeline.shader, "model", model);
                        glCmd->setUniformVec3(forwardPipeline.shader, "color", obj.material.albedo);
                        glCmd->setUniformFloat(forwardPipeline.shader, "alpha", 0.4f);
                        glCmd->bindVertexArray(obj.mesh->VAO);
                        glCmd->drawIndexed((unsigned int)obj.mesh->indices.size());
                    }
                }
            }
        );

        renderGraph.addPass<LightingData>("UI",
            [&](RGBuilder& builder, LightingData& data) {},
            [&](LightingData& data, RGRegistry& registry, ICommandBuffer* cmd) {
                GLCommandBuffer* glCmd = static_cast<GLCommandBuffer*>(cmd);
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