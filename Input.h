#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Input {
public:
    static void Initialize(GLFWwindow* window);
    static void Update();

    static bool IsKeyDown(int key);
    static bool IsMouseButtonDown(int button);

    static float GetMouseX();
    static float GetMouseY();
    static float GetScrollY();
    static void SetCursorMode(bool visible);
    static void SetCursorPos(float x, float y);
private:
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static GLFWwindow* windowHandle;
    static float scrollY;
};