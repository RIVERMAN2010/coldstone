#include "Camera.h"
#include "Input.h"
#include "Utils.h"
#include <algorithm>

Camera::Camera(glm::vec3 startPos)
    : Position(startPos), Pivot(0.0f, 0.0f, 0.0f), WorldUp(0.0f, 1.0f, 0.0f),
    Yaw(-90.0f), Pitch(0.0f), Front(0.0f, 0.0f, -1.0f),
    MovementSpeed(10.0f), MouseSensitivity(0.1f), Fov(45.0f),
    flyMode(false), Distance(5.0f), firstMouse(true)
{
    Pivot = Position + Front * Distance;
    UpdateVectors();
}

void Camera::ToggleFlightMode() {
    flyMode = !flyMode;
    firstMouse = true;

    if (flyMode) {
        Input::SetCursorMode(false);
        Log("Switched to Fly Mode (WASD + Mouse)");
    }
    else {
        Input::SetCursorMode(true);
        Pivot = Position + Front * 5.0f;
        Distance = 5.0f;
        Log("Switched to Orbit Mode (MMB=Rotate, Shift+MMB=Pan, G=Fly)");
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
    static bool gPressed = false;
    if (Input::IsKeyDown(GLFW_KEY_G)) {
        if (!gPressed) { ToggleFlightMode(); gPressed = true; }
    }
    else { gPressed = false; }

    if (flyMode)
        UpdateFly(deltaTime);
    else
        UpdateOrbit(deltaTime, screenW, screenH);
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
    float xpos = Input::GetMouseX();
    float ypos = Input::GetMouseY();

    if (firstMouse) { lastMouseX = xpos; lastMouseY = ypos; firstMouse = false; }

    bool wrapped = false;
    if (Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
        if (xpos <= 1) {
            Input::SetCursorPos((float)screenW - 2, ypos);
            lastMouseX = (float)screenW - 2;
            wrapped = true;
        }
        else if (xpos >= screenW - 1) {
            Input::SetCursorPos(2, ypos);
            lastMouseX = 2;
            wrapped = true;
        }

        if (ypos <= 1) {
            Input::SetCursorPos(xpos, (float)screenH - 2);
            lastMouseY = (float)screenH - 2;
            wrapped = true;
        }
        else if (ypos >= screenH - 1) {
            Input::SetCursorPos(xpos, 2);
            lastMouseY = 2;
            wrapped = true;
        }
    }
    if (wrapped) return;

    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos;
    lastMouseX = xpos; lastMouseY = ypos;

    float scroll = Input::GetScrollY();
    if (scroll != 0.0f) {
        Distance -= scroll * 2.0f;
        if (Distance < 1.0f) Distance = 1.0f;
    }

    if (Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
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
    Position = Pivot - (Front * Distance);
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix(int screenW, int screenH) {
    return glm::perspective(glm::radians(Fov), (float)screenW / (float)screenH, 0.1f, 100.0f);
}