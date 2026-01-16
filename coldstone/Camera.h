#pragma once

#include <glm/glm.hpp>

class Camera3D
{
public:
    Camera3D(
        glm::vec3 position,
        float fovDegrees,
        float aspectRatio,
        float nearPlane,
        float farPlane
    );

    // --- Movement ---
    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);

    // --- Rotation ---
    void rotate(float yawOffset, float pitchOffset);

    // --- Setters ---
    void setPosition(const glm::vec3& pos);
    void setAspectRatio(float aspect);

    // --- Getters ---
    glm::vec3 getPosition() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

private:
    void updateVectors();

private:
    glm::vec3 position;

    float yaw;     // degrees
    float pitch;   // degrees

    float fov;
    float aspect;
    float nearPlane;
    float farPlane;

    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };
};