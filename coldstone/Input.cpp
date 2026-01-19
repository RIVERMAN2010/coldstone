#include "Input.h"
#include <cstring>

GLFWwindow* Input::windowHandle = nullptr;
bool Input::keys[GLFW_KEY_LAST] = { false };
bool Input::keysPressed[GLFW_KEY_LAST] = { false };
bool Input::mouseButtons[3] = { false };
bool Input::mouseButtonsPressed[3] = { false };
float Input::scrollY = 0.0f;
double Input::mouseX = 0.0;
double Input::mouseY = 0.0;

void Input::Initialize(GLFWwindow* window) {
    windowHandle = window;
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
}

void Input::Update() {
    memset(keysPressed, 0, sizeof(keysPressed));
    memset(mouseButtonsPressed, 0, sizeof(mouseButtonsPressed));
    scrollY = 0.0f;
}

void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key < 0 || key >= GLFW_KEY_LAST) return;
    if (action == GLFW_PRESS) {
        keys[key] = true;
        keysPressed[key] = true;
    }
    else if (action == GLFW_RELEASE) {
        keys[key] = false;
        keysPressed[key] = false;
    }
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button < 0 || button >= 3) return;
    if (action == GLFW_PRESS) {
        mouseButtons[button] = true;
        mouseButtonsPressed[button] = true;
    }
    else if (action == GLFW_RELEASE) {
        mouseButtons[button] = false;
    }
}

void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    scrollY = (float)yoffset;
}

void Input::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;
}

bool Input::IsKeyPressed(int key) { return keysPressed[key]; }
bool Input::IsKeyDown(int key) { return keys[key]; }
bool Input::IsMouseButtonPressed(int button) { return mouseButtonsPressed[button]; }
bool Input::IsMouseButtonDown(int button) { return mouseButtons[button]; }

float Input::GetMouseX() { return (float)mouseX; }
float Input::GetMouseY() { return (float)mouseY; }
float Input::GetScrollY() { return scrollY; }

void Input::SetCursorMode(bool visible) {
    glfwSetInputMode(windowHandle, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
void Input::SetCursorPos(float x, float y) {
    glfwSetCursorPos(windowHandle, (double)x, (double)y);
}