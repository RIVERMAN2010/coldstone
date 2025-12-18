#include "Camera2D.h"

Camera2D::Camera2D(float left, float right, float bottom, float top)
{
    m_Projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    RecalculateViewProjection();
}

void Camera2D::SetProjection(float left, float right, float bottom, float top)
{
    m_Projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    RecalculateViewProjection();
}

void Camera2D::SetPosition(const glm::vec2& position)
{
    m_Position = position;
    RecalculateViewProjection();
}

void Camera2D::SetRotation(float rotationRadians)
{
    m_Rotation = rotationRadians;
    RecalculateViewProjection();
}

void Camera2D::SetZoom(float zoom)
{
    m_Zoom = zoom;
    RecalculateViewProjection();
}

const glm::vec2& Camera2D::GetPosition() const { return m_Position; }
float Camera2D::GetRotation() const { return m_Rotation; }
float Camera2D::GetZoom() const { return m_Zoom; }

const glm::mat4& Camera2D::GetViewProjection() const
{
    return m_ViewProjection;
}

void Camera2D::RecalculateViewProjection()
{
    // Camera transform (inverse of world transform)
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(m_Position, 0.0f))
        * glm::rotate(glm::mat4(1.0f), m_Rotation, glm::vec3(0, 0, 1))
        * glm::scale(glm::mat4(1.0f), glm::vec3(m_Zoom, m_Zoom, 1.0f));

    m_View = glm::inverse(transform);
    m_ViewProjection = m_Projection * m_View;
}