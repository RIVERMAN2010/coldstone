#include "Input.h"

GLFWwindow* Input::windowHandle = nullptr;
float Input::scrollY = 0.0f;

void Input::Initialize(GLFWwindow* window) {
    windowHandle = window;
    glfwSetScrollCallback(window, ScrollCallback);
}

void Input::Update() {
    scrollY = 0.0f;
}

bool Input::IsKeyDown(int key) {
    return glfwGetKey(windowHandle, key) == GLFW_PRESS;
}

bool Input::IsMouseButtonDown(int button) {
    return glfwGetMouseButton(windowHandle, button) == GLFW_PRESS;
}

float Input::GetMouseX() {
    double x, y;
    glfwGetCursorPos(windowHandle, &x, &y);
    return (float)x;
}

float Input::GetMouseY() {
    double x, y;
    glfwGetCursorPos(windowHandle, &x, &y);
    return (float)y;
}

float Input::GetScrollY() {
    return scrollY;
}

void Input::SetCursorMode(bool visible) {
    if (visible) {
        glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
        glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void Input::SetCursorPos(float x, float y) {
    glfwSetCursorPos(windowHandle, (double)x, (double)y);
}

void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    scrollY = (float)yoffset;
}