#include "Material.h"

namespace Metaphysics {

Material::Material()
    : m_Ambient(glm::vec3(0.4f))     // 增加环境光
    , m_Diffuse(glm::vec3(0.9f))     // 增加漫反射
    , m_Specular(glm::vec3(0.5f))    // 适度的高光
    , m_Shininess(32.0f)
{
}

Material::Material(const glm::vec3& ambient, const glm::vec3& diffuse,
                   const glm::vec3& specular, float shininess)
    : m_Ambient(ambient)
    , m_Diffuse(diffuse)
    , m_Specular(specular)
    , m_Shininess(shininess)
{
}

} // namespace Metaphysics
