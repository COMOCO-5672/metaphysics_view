#pragma once

#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

namespace Metaphysics {

// OpenGL着色器类
class Shader {
public:
    Shader() : m_ProgramID(0) {}
    ~Shader();

    bool LoadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc);
    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    
    void Use() const;
    void Unbind() const;
    
    uint32_t GetProgramID() const { return m_ProgramID; }

    // Uniform设置
    void SetBool(const std::string& name, bool value);
    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec2(const std::string& name, const glm::vec2& value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetMat3(const std::string& name, const glm::mat3& value);
    void SetMat4(const std::string& name, const glm::mat4& value);

private:
    uint32_t CompileShader(uint32_t type, const std::string& source);
    int GetUniformLocation(const std::string& name);

private:
    uint32_t m_ProgramID;
    std::unordered_map<std::string, int> m_UniformLocationCache;
};

} // namespace Metaphysics
