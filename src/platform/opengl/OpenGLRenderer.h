#pragma once

// GLEW must be included before any OpenGL headers
#include <GL/glew.h>

#include "../../core/renderer/RenderAPI.h"
#include "../../core/scene/Scene.h"
#include "../../core/camera/Camera.h"
#include "Shader.h"
#include <memory>

namespace Metaphysics {

enum class RenderMode {
    Solid,
    Wireframe,
    SolidWireframe
};

struct LightSettings {
    glm::vec3 position    = glm::vec3(5.0f, 8.0f, 5.0f);
    glm::vec3 color       = glm::vec3(1.0f, 1.0f, 1.0f);
    float ambientStrength = 0.3f;
    float diffuseStrength = 1.0f;
    float specularStrength = 0.5f;
};

struct RenderSettings {
    LightSettings light;
    RenderMode renderMode = RenderMode::Solid;
    glm::vec4 clearColor  = glm::vec4(0.15f, 0.15f, 0.18f, 1.0f);
    glm::vec3 wireColor   = glm::vec3(1.0f, 0.5f, 0.0f);

    bool showGrid = true;
    bool showAxes = true;
    float gridSize = 10.0f;
    float gridSpacing = 1.0f;
};

class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    bool Init();
    void Shutdown();

    void BeginFrame(const glm::vec4& clearColor);
    void EndFrame();
    void RenderScene(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera,
                     const RenderSettings& settings);

    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    std::shared_ptr<Entity> PickEntity(std::shared_ptr<Scene> scene,
                                       std::shared_ptr<Camera> camera,
                                       int mouseX, int mouseY,
                                       int screenWidth, int screenHeight);

private:
    void SetupMesh(std::shared_ptr<Mesh> mesh);
    void RenderMesh(std::shared_ptr<Mesh> mesh, const glm::mat4& transform);
    void CleanupMesh(std::shared_ptr<Mesh> mesh);

    void SetupGrid(float size, float spacing);
    void RenderGrid(const glm::mat4& view, const glm::mat4& projection,
                    float size, float spacing);
    void RenderAxes(const glm::mat4& view, const glm::mat4& projection);

    bool RayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                              const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                              float& t);

private:
    std::unique_ptr<Shader> m_Shader;
    std::unique_ptr<Shader> m_WireframeShader;
    std::unique_ptr<Shader> m_GridShader;

    uint32_t m_GridVAO = 0;
    uint32_t m_GridVBO = 0;
    uint32_t m_GridVertexCount = 0;
    float m_LastGridSize = 0.0f;
    float m_LastGridSpacing = 0.0f;

    uint32_t m_AxesVAO = 0;
    uint32_t m_AxesVBO = 0;

    uint32_t m_ViewportWidth;
    uint32_t m_ViewportHeight;
};

} // namespace Metaphysics
