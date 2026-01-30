#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Metaphysics {

Camera::Camera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch)
    : m_Position(position)
    , m_WorldUp(up)
    , m_Yaw(yaw)
    , m_Pitch(pitch)
    , m_MovementSpeed(2.5f)
    , m_MouseSensitivity(0.1f)
    , m_Zoom(45.0f)
    , m_Near(0.1f)
    , m_Far(100.0f)
{
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const
{
    return glm::perspective(glm::radians(m_Zoom), aspectRatio, m_Near, m_Far);
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = m_MovementSpeed * deltaTime;
    
    switch (direction) {
        case CameraMovement::FORWARD:
            m_Position += m_Front * velocity;
            break;
        case CameraMovement::BACKWARD:
            m_Position -= m_Front * velocity;
            break;
        case CameraMovement::LEFT:
            m_Position -= m_Right * velocity;
            break;
        case CameraMovement::RIGHT:
            m_Position += m_Right * velocity;
            break;
        case CameraMovement::UP:
            m_Position += m_WorldUp * velocity;
            break;
        case CameraMovement::DOWN:
            m_Position -= m_WorldUp * velocity;
            break;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    if (constrainPitch) {
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;
    }

    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    m_Zoom -= yoffset;
    if (m_Zoom < 1.0f)
        m_Zoom = 1.0f;
    if (m_Zoom > 45.0f)
        m_Zoom = 45.0f;
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);
    
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

} // namespace Metaphysics
