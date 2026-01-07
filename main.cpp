#include "Utils.h"
#include "MainOpenGL.h"
#include "RenderFont.h"
#include "MainDevIL.h"
#include "Input.h"
#include "Camera.h"
#include "Scene.h" 
#include <chrono>
#include <thread>
#include <format>

int main() {
    int screenW = GetSystemMetrics(SM_CXMAXTRACK);
    int screenH = GetSystemMetrics(SM_CYMAXTRACK);

    MainOpenGL::Initialize(screenW, screenH, "Engine Start");
    MainDevIL::Initialize();
    RenderFont::Initialize();
    Input::Initialize(MainOpenGL::GetWindow());

    std::string exeDir = GetExeDirectory();
    AssetLoader::AddAssetDir(exeDir);
    AssetLoader::AddAssetDir(exeDir + "/Assets");

    Scene::Initialize();
    Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double lastTime = 0.0;
    int nbFrames = 0;
    int currentFPS = 0;
    const double targetFrameTime = 1.0 / 60.0;

    while (!MainOpenGL::ShouldClose()) {
        double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()) / 1e9;
        double frameStartTime = currentTime;

        MainOpenGL::PollEvents();
        MainOpenGL::GetWindowSize(screenW, screenH);

        static double lastFrameT = 0.0;
        double dt = currentTime - lastFrameT;
        lastFrameT = currentTime;
        camera.Update((float)dt, screenW, screenH);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Scene::Draw(camera.GetViewMatrix(), camera.GetProjectionMatrix(screenW, screenH), camera.GetPosition());

        RenderFont::Draw(screenW, screenH);

        MainOpenGL::SwapBuffers();
        Input::Update();

        double endTime = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()) / 1e9;
        double workTime = endTime - frameStartTime;
        if (workTime < targetFrameTime) {
            double sleepS = targetFrameTime - workTime - 0.002;
            if (sleepS > 0) std::this_thread::sleep_for(std::chrono::milliseconds((int)(sleepS * 1000)));
            while ((static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()) / 1e9) - frameStartTime < targetFrameTime);
        }

        nbFrames++;
        double now = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()) / 1e9;
        if (now - lastTime >= 1.0) {
            currentFPS = nbFrames;
            RenderFont::UpdateText(std::format("FPS: {}", currentFPS));
            nbFrames = 0;
            lastTime = now;
        }
    }

    Scene::Cleanup();
    RenderFont::Cleanup();
    MainOpenGL::Cleanup();
    return 0;
}