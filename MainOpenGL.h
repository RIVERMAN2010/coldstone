#pragma once
#include <Windows.h>

struct GLFWwindow;

class MainOpenGL {
public:
    static void Initialize(int width, int height, const char* title);
    static bool ShouldClose();
    static void SwapBuffers();
    static void PollEvents();
    static void Cleanup();
    static void GetWindowSize(int& width, int& height);
    static void* GetNativeWindow();
    static GLFWwindow* GetWindow();

private:
    static GLFWwindow* window;
};