#include "Material.h"

namespace Metaphysics {

Material::Material()
    : m_Ambient(glm::vec3(0.2f))
    , m_Diffuse(glm::vec3(0.8f))
    , m_Specular(glm::vec3(1.0f))
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
