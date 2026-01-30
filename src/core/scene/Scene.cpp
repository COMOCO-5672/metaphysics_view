#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace Metaphysics {

// Entity implementation
uint32_t Entity::s_NextID = 1;

Entity::Entity(const std::string& name, std::shared_ptr<Model> model)
    : m_ID(s_NextID++)
    , m_Name(name)
    , m_Model(model)
    , m_Position(0.0f)
    , m_Rotation(0.0f)
    , m_Scale(1.0f)
    , m_Transform(1.0f)
    , m_Visible(true)
    , m_Selected(false)
{
    UpdateTransform();
}

void Entity::UpdateTransform()
{
    m_Transform = glm::mat4(1.0f);
    m_Transform = glm::translate(m_Transform, m_Position);
    m_Transform = glm::rotate(m_Transform, glm::radians(m_Rotation.x), glm::vec3(1, 0, 0));
    m_Transform = glm::rotate(m_Transform, glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));
    m_Transform = glm::rotate(m_Transform, glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));
    m_Transform = glm::scale(m_Transform, m_Scale);
}

// Scene implementation
Scene::Scene(const std::string& name)
    : m_Name(name)
{
}

std::shared_ptr<Entity> Scene::CreateEntity(const std::string& name, std::shared_ptr<Model> model)
{
    auto entity = std::make_shared<Entity>(name, model);
    m_Entities.push_back(entity);
    return entity;
}

void Scene::RemoveEntity(uint32_t entityID)
{
    m_Entities.erase(
        std::remove_if(m_Entities.begin(), m_Entities.end(),
            [entityID](const std::shared_ptr<Entity>& entity) {
                return entity->GetID() == entityID;
            }),
        m_Entities.end()
    );
}

void Scene::RemoveEntity(std::shared_ptr<Entity> entity)
{
    if (entity) {
        RemoveEntity(entity->GetID());
    }
}

void Scene::Clear()
{
    m_Entities.clear();
}

std::shared_ptr<Entity> Scene::GetEntity(uint32_t entityID)
{
    for (auto& entity : m_Entities) {
        if (entity->GetID() == entityID) {
            return entity;
        }
    }
    return nullptr;
}

} // namespace Metaphysics
