#pragma once

#include "../../core/renderer/RenderAPI.h"
#include "../../core/scene/Scene.h"
#include "../../core/camera/Camera.h"
#include "Shader.h"
#include <memory>

namespace Metaphysics {

// OpenGL渲染器 - 负责实际的渲染工作
class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    bool Init();
    void Shutdown();

    // 渲染场景
    void BeginFrame(const glm::vec4& clearColor);
    void EndFrame();
    void RenderScene(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera);

    // 设置视口
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    // 拾取功能
    std::shared_ptr<Entity> PickEntity(std::shared_ptr<Scene> scene, 
                                       std::shared_ptr<Camera> camera,
                                       int mouseX, int mouseY, 
                                       int screenWidth, int screenHeight);

private:
    void SetupMesh(std::shared_ptr<Mesh> mesh);
    void RenderMesh(std::shared_ptr<Mesh> mesh, const glm::mat4& transform);
    void CleanupMesh(std::shared_ptr<Mesh> mesh);

    // 射线与三角形相交测试
    bool RayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                              const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                              float& t);

private:
    std::unique_ptr<Shader> m_Shader;
    std::unique_ptr<Shader> m_WireframeShader;
    
    uint32_t m_ViewportWidth;
    uint32_t m_ViewportHeight;
};

} // namespace Metaphysics
