#pragma once

// GLEW must be included before any OpenGL headers
#include <GL/glew.h>

#include "../../core/renderer/ISceneRenderer.h"
#include "Shader.h"
#include <memory>

namespace Metaphysics {

class OpenGLRenderer : public ISceneRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    bool Init(void* window) override;
    void Shutdown() override;

    void BeginFrame(const glm::vec4& clearColor) override;
    void EndFrame() override;
    void RenderScene(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera,
                     const RenderSettings& settings) override;

    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

    std::shared_ptr<Entity> PickEntity(std::shared_ptr<Scene> scene,
                                       std::shared_ptr<Camera> camera,
                                       int mouseX, int mouseY,
                                       int screenWidth, int screenHeight) override;

    void RenderBlenderUI(const BlenderDrawList& drawList) override;

    UIRendererContext GetUIContext() const override;

private:
    void SetupMesh(std::shared_ptr<Mesh> mesh);
    void RenderMesh(std::shared_ptr<Mesh> mesh, const glm::mat4& transform);
    void CleanupMesh(std::shared_ptr<Mesh> mesh);

    void SetupGrid(float size, float spacing, bool colorNegativeAxes, bool showAxisX, bool showAxisZ);
    void RenderGrid(const glm::mat4& view, const glm::mat4& projection,
                    float size, float spacing, bool colorNegativeAxes, bool showAxisX, bool showAxisZ);
    void RenderAxes(const glm::mat4& view, const glm::mat4& projection, const RenderSettings& settings);
    void EnsureUIResources();

    bool RayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                              const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                              float& t);

private:
    std::unique_ptr<Shader> m_Shader;
    std::unique_ptr<Shader> m_WireframeShader;
    std::unique_ptr<Shader> m_GridShader;
    std::unique_ptr<Shader> m_UIShader;

    uint32_t m_GridVAO = 0;
    uint32_t m_GridVBO = 0;
    uint32_t m_GridVertexCount = 0;
    float m_LastGridSize = 0.0f;
    float m_LastGridSpacing = 0.0f;
    bool m_LastGridColorNegativeAxes = true;
    bool m_LastShowAxisXOnGrid = true;
    bool m_LastShowAxisZOnGrid = true;

    uint32_t m_AxesVAO = 0;
    uint32_t m_AxesVBO = 0;
    uint32_t m_UIVAO = 0;
    uint32_t m_UIVBO = 0;

    uint32_t m_ViewportWidth;
    uint32_t m_ViewportHeight;
};

} // namespace Metaphysics
