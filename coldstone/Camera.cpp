#include "Camera.h"
#include "Input.h"
#include <algorithm>
#include <iostream>

Camera::Camera(glm::vec3 startPos)
    : Position(startPos), Pivot(0.0f, 0.0f, 0.0f), WorldUp(0.0f, 1.0f, 0.0f),
    Yaw(-90.0f), Pitch(0.0f), Front(0.0f, 0.0f, -1.0f),
    MovementSpeed(10.0f), MouseSensitivity(0.1f), Fov(45.0f),
    flyMode(false), Distance(10.0f), firstMouse(true)
{
    Pivot = glm::vec3(0, 0, 0);
    UpdateVectors();
}

void Camera::ToggleFlightMode() {
    flyMode = !flyMode;
    firstMouse = true;
    if (flyMode) {
        Input::SetCursorMode(false);
    }
    else {
        Input::SetCursorMode(true);
    }
}

void Camera::UpdateVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::Update(float deltaTime, int screenW, int screenH) {
    static bool fPressed = false;
    if (Input::IsKeyDown(GLFW_KEY_F)) {
        if (!fPressed) { ToggleFlightMode(); fPressed = true; }
    }
    else { fPressed = false; }

    if (flyMode) UpdateFly(deltaTime);
    else UpdateOrbit(deltaTime, screenW, screenH);
}

void Camera::UpdateFly(float deltaTime) {
    float xpos = Input::GetMouseX();
    float ypos = Input::GetMouseY();

    if (firstMouse) { lastMouseX = xpos; lastMouseY = ypos; firstMouse = false; }

    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos;
    lastMouseX = xpos; lastMouseY = ypos;

    Yaw += xoffset * MouseSensitivity;
    Pitch += yoffset * MouseSensitivity;

    if (Pitch > 89.0f) Pitch = 89.0f;
    if (Pitch < -89.0f) Pitch = -89.0f;

    UpdateVectors();

    float velocity = MovementSpeed * deltaTime;
    if (Input::IsKeyDown(GLFW_KEY_W)) Position += Front * velocity;
    if (Input::IsKeyDown(GLFW_KEY_S)) Position -= Front * velocity;
    if (Input::IsKeyDown(GLFW_KEY_A)) Position -= Right * velocity;
    if (Input::IsKeyDown(GLFW_KEY_D)) Position += Right * velocity;
    if (Input::IsKeyDown(GLFW_KEY_SPACE)) Position += WorldUp * velocity;
    if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT)) Position -= WorldUp * velocity;
}

void Camera::UpdateOrbit(float deltaTime, int screenW, int screenH) {
    float scroll = Input::GetScrollY();
    if (scroll != 0.0f) {
        Distance -= scroll * 2.0f;
        if (Distance < 1.0f) Distance = 1.0f;
    }

    if (Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
        float xpos = Input::GetMouseX();
        float ypos = Input::GetMouseY();
        if (firstMouse) { lastMouseX = xpos; lastMouseY = ypos; firstMouse = false; }

        float xoffset = xpos - lastMouseX;
        float yoffset = lastMouseY - ypos;
        lastMouseX = xpos; lastMouseY = ypos;

        if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
            float panSpeed = Distance * 0.002f;
            Pivot -= Right * (xoffset * panSpeed);
            Pivot -= Up * (yoffset * panSpeed);
        }
        else {
            Yaw += xoffset * MouseSensitivity;
            Pitch += yoffset * MouseSensitivity;
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
            UpdateVectors();
        }
    }
    else {
        firstMouse = true;
    }
    Position = Pivot - (Front * Distance);
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix(int screenW, int screenH) {
    return glm::perspective(glm::radians(Fov), (float)screenW / (float)screenH, 0.1f, 100.0f);
}