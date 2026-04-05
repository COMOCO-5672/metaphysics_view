#pragma once

#include <glm/glm.hpp>

namespace Metaphysics {

// 相机移动方向枚举
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// 相机类
class Camera {
public:
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f);

    // 获取视图矩阵
    glm::mat4 GetViewMatrix() const;
    
    // 获取投影矩阵
    glm::mat4 GetProjectionMatrix(float aspectRatio) const;

    // 相机移动
    void ProcessKeyboard(CameraMovement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    // Getters
    const glm::vec3& GetPosition() const { return m_Position; }
    const glm::vec3& GetFront() const { return m_Front; }
    const glm::vec3& GetUp() const { return m_Up; }
    float GetZoom() const { return m_Zoom; }
    float GetNearPlane() const { return m_Near; }
    float GetFarPlane() const { return m_Far; }

    // Setters
    void SetPosition(const glm::vec3& position) { m_Position = position; }
    void LookAt(const glm::vec3& target);
    void SetZoom(float zoom);
    void SetNearFar(float nearPlane, float farPlane) { m_Near = nearPlane; m_Far = farPlane; }

private:
    void UpdateCameraVectors();

private:
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    float m_Yaw;
    float m_Pitch;
    float m_MovementSpeed;
    float m_MouseSensitivity;
    float m_Zoom;
    float m_Near;
    float m_Far;
};

} // namespace Metaphysics
