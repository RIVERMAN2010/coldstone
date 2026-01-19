#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Input {
public:
    static void Initialize(GLFWwindow* window);
    static void Update();

    static bool IsKeyPressed(int key);
    static bool IsKeyDown(int key);

    static bool IsMouseButtonPressed(int button);
    static bool IsMouseButtonDown(int button);

    static float GetMouseX();
    static float GetMouseY();
    static float GetScrollY();
    static void SetCursorMode(bool visible);
    static void SetCursorPos(float x, float y);

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);

private:
    static GLFWwindow* windowHandle;
    static bool keys[GLFW_KEY_LAST];
    static bool keysPressed[GLFW_KEY_LAST];
    static bool mouseButtons[3];
    static bool mouseButtonsPressed[3];
    static float scrollY;
    static double mouseX, mouseY;
};