#include "MainOpenGL.h"
#include "Utils.h"
#include <GL/glew.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <format>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "glew32.lib")

GLFWwindow* MainOpenGL::window = nullptr;

void MainOpenGL::Initialize(int width, int height, const char* title) {
    if (!glfwInit()) {
        Log("Failed to initialize GLFW.");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        Log("Failed to create GLFW window.");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        Log("Failed to initialize GLEW.");
        return;
    }

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    Log(std::format("OpenGL Initialized: {}", (const char*)glGetString(GL_VERSION)));
}

bool MainOpenGL::ShouldClose() {
    return glfwWindowShouldClose(window);
}

void MainOpenGL::SwapBuffers() {
    glfwSwapBuffers(window);
}

void MainOpenGL::PollEvents() {
    glfwPollEvents();
}

void MainOpenGL::Cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void MainOpenGL::GetWindowSize(int& width, int& height) {
    glfwGetWindowSize(window, &width, &height);
}

void* MainOpenGL::GetNativeWindow() {
    return glfwGetWin32Window(window);
}

GLFWwindow* MainOpenGL::GetWindow() {
    return window;
}
