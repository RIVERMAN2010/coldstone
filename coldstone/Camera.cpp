#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera3D::Camera3D(glm::vec3 position, float fovDegrees, float aspectRatio, float nearPlane, float farPlane) :
    position(position), yaw(-90.0f), pitch(0.0f), fov(fovDegrees), aspect(aspectRatio), nearPlane(nearPlane), farPlane(farPlane) {
    updateVectors();
}

void Camera3D::moveForward(float amount) {
    position += forward * amount;
}

void Camera3D::moveRight(float amount) {
    position += right * amount;
}

void Camera3D::moveUp(float amount) {
    position += worldUp * amount;
}

void Camera3D::rotate(float yawOffset, float pitchOffset) {
    yaw += yawOffset;
    pitch += pitchOffset;

    pitch = std::clamp(pitch, -89.0f, 89.0f);

    updateVectors();
}

void Camera3D::updateVectors() {
    glm::vec3 direction;

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    forward = glm::normalize(direction);
    right = glm::normalize(glm::cross(forward, worldUp));
    up = glm::normalize(glm::cross(right, forward));
}

glm::mat4 Camera3D::getViewMatrix() const {
    return glm::lookAt(position, position + forward, up);
}

glm::mat4 Camera3D::getProjectionMatrix() const {
    return glm::perspective(
        glm::radians(fov),
        aspect,
        nearPlane,
        farPlane
    );
}

void Camera3D::setPosition(const glm::vec3& pos) {
    position = pos;
}

void Camera3D::setAspectRatio(float aspectRatio) {
    aspect = aspectRatio;
}

glm::vec3 Camera3D::getPosition() const {
    return position;
}

glm::vec3 Camera3D::getForward() const {
    return forward;
}

glm::vec3 Camera3D::getRight() const {
    return right;
}

glm::vec3 Camera3D::getUp() const {
    return up;
}