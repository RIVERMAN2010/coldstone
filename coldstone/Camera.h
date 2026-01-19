#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(glm::vec3 startPos);

    void Update(float deltaTime, int screenW, int screenH);
    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix(int screenW, int screenH);
    glm::vec3 GetPosition() const { return Position; }

    void ToggleFlightMode();
    bool IsInFlightMode() const { return flyMode; }

private:
    void UpdateOrbit(float deltaTime, int screenW, int screenH);
    void UpdateFly(float deltaTime);
    void UpdateVectors();

    glm::vec3 Position;
    glm::vec3 Pivot;
    glm::vec3 Front, Up, Right, WorldUp;

    float Yaw, Pitch;
    float Distance;

    float MovementSpeed;
    float MouseSensitivity;
    float Fov;
    bool flyMode;

    float lastMouseX, lastMouseY;
    bool firstMouse;
};