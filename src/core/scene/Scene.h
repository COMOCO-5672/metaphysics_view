#pragma once

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "../model/Model.h"

namespace Metaphysics {

// 场景中的实体（模型实例）
class Entity {
public:
    Entity(const std::string& name, std::shared_ptr<Model> model);

    // Transform操作
    void SetPosition(const glm::vec3& position) { m_Position = position; UpdateTransform(); }
    void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; UpdateTransform(); }
    void SetScale(const glm::vec3& scale) { m_Scale = scale; UpdateTransform(); }

    const glm::vec3& GetPosition() const { return m_Position; }
    const glm::vec3& GetRotation() const { return m_Rotation; }
    const glm::vec3& GetScale() const { return m_Scale; }

    const glm::mat4& GetTransform() const { return m_Transform; }
    
    // 模型访问
    std::shared_ptr<Model> GetModel() const { return m_Model; }
    
    // 名称和可见性
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }
    
    bool IsVisible() const { return m_Visible; }
    void SetVisible(bool visible) { m_Visible = visible; }

    // 选中状态
    bool IsSelected() const { return m_Selected; }
    void SetSelected(bool selected) { m_Selected = selected; }

    // 获取唯一ID
    uint32_t GetID() const { return m_ID; }

private:
    void UpdateTransform();

private:
    static uint32_t s_NextID;
    uint32_t m_ID;
    
    std::string m_Name;
    std::shared_ptr<Model> m_Model;
    
    glm::vec3 m_Position;
    glm::vec3 m_Rotation;
    glm::vec3 m_Scale;
    glm::mat4 m_Transform;
    
    bool m_Visible;
    bool m_Selected;
};

// 场景类 - 管理所有实体
class Scene {
public:
    Scene(const std::string& name = "Untitled Scene");

    // 实体管理
    std::shared_ptr<Entity> CreateEntity(const std::string& name, std::shared_ptr<Model> model);
    void RemoveEntity(uint32_t entityID);
    void RemoveEntity(std::shared_ptr<Entity> entity);
    void Clear();

    // 获取实体
    std::shared_ptr<Entity> GetEntity(uint32_t entityID);
    const std::vector<std::shared_ptr<Entity>>& GetEntities() const { return m_Entities; }

    // 场景名称
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

private:
    std::string m_Name;
    std::vector<std::shared_ptr<Entity>> m_Entities;
};

} // namespace Metaphysics
