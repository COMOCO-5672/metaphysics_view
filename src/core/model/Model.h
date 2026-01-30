#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "../renderer/RenderAPI.h"
#include "../material/Material.h"

namespace Metaphysics {

// 网格数据结构
struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::shared_ptr<Material> material;
    
    // 用于渲染的GPU资源ID（OpenGL的VAO/VBO/EBO等）
    uint32_t VAO = 0;
    uint32_t VBO = 0;
    uint32_t EBO = 0;

    // AABB包围盒，用于拾取和碰撞检测
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
};

// 模型类 - 包含多个网格
class Model {
public:
    Model() = default;
    ~Model() = default;

    // 加载模型（使用Assimp）
    bool LoadFromFile(const std::string& path);
    
    // 获取网格列表
    const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const { return m_Meshes; }
    std::vector<std::shared_ptr<Mesh>>& GetMeshes() { return m_Meshes; }
    
    // 获取模型名称
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

    // 获取模型路径
    const std::string& GetPath() const { return m_Path; }

    // 计算模型的AABB
    void ComputeAABB(glm::vec3& outMin, glm::vec3& outMax) const;

private:
    void ProcessNode(void* node, void* scene);
    std::shared_ptr<Mesh> ProcessMesh(void* mesh, void* scene);
    void ComputeMeshAABB(std::shared_ptr<Mesh> mesh);

private:
    std::vector<std::shared_ptr<Mesh>> m_Meshes;
    std::string m_Name;
    std::string m_Path;
    std::string m_Directory;
};

} // namespace Metaphysics
