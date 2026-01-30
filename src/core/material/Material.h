#pragma once

#include <glm/glm.hpp>

namespace Metaphysics {

// 材质类
class Material {
public:
    Material();
    Material(const glm::vec3& ambient, const glm::vec3& diffuse, 
             const glm::vec3& specular, float shininess);

    // Getters
    const glm::vec3& GetAmbient() const { return m_Ambient; }
    const glm::vec3& GetDiffuse() const { return m_Diffuse; }
    const glm::vec3& GetSpecular() const { return m_Specular; }
    float GetShininess() const { return m_Shininess; }

    // Setters
    void SetAmbient(const glm::vec3& ambient) { m_Ambient = ambient; }
    void SetDiffuse(const glm::vec3& diffuse) { m_Diffuse = diffuse; }
    void SetSpecular(const glm::vec3& specular) { m_Specular = specular; }
    void SetShininess(float shininess) { m_Shininess = shininess; }

private:
    glm::vec3 m_Ambient;
    glm::vec3 m_Diffuse;
    glm::vec3 m_Specular;
    float m_Shininess;
};

} // namespace Metaphysics
