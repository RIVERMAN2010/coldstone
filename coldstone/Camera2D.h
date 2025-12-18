#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera2D {
public:
    Camera2D(float left, float right, float bottom, float top);

    // Setters
    void SetPosition(const glm::vec2& position);
    void SetRotation(float rotationRadians);
    void SetZoom(float zoom);

    // Getters
    const glm::vec2& GetPosition() const;
    float GetRotation() const;
    float GetZoom() const;

    // Matrices
    const glm::mat4& GetViewProjection() const;

    // Projection resize (window resize)
    void SetProjection(float left, float right, float bottom, float top);

private:
    void RecalculateViewProjection();

private:
    glm::mat4 m_Projection{ 1.0f };
    glm::mat4 m_View{ 1.0f };
    glm::mat4 m_ViewProjection{ 1.0f };

    glm::vec2 m_Position{ 0.0f, 0.0f };
    float m_Rotation = 0.0f; // radians
    float m_Zoom = 1.0f;
};