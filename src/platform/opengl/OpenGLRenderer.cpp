#include "OpenGLRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

namespace Metaphysics {

// ============================================================
// Phong lighting shader — ambient/diffuse/specular strengths
// are passed as uniforms so the UI can tweak them at runtime.
// ============================================================
static const char* phongVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

static const char* phongFragmentShader = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;

uniform vec3 material_ambient;
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;

void main()
{
    // Ambient — uses a strength factor so even dark materials remain visible
    vec3 ambient = ambientStrength * lightColor * material_ambient;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * lightColor * (diff * material_diffuse);

    // Specular (Blinn-Phong half-vector for smoother highlights)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), material_shininess);
    vec3 specular = specularStrength * lightColor * (spec * material_specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
)";

// ============================================================
// Simple colored-line shader (wireframe / grid / axes)
// ============================================================
static const char* lineVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vColor = aColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

static const char* lineFragmentShader = R"(
#version 330 core
out vec4 FragColor;
in vec3 vColor;

uniform float alpha;

void main()
{
    FragColor = vec4(vColor, alpha);
}
)";

// Wireframe overlay uses the old simple shader (uniform color)
static const char* wireframeVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

static const char* wireframeFragmentShader = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 wireframeColor;

void main()
{
    FragColor = vec4(wireframeColor, 1.0);
}
)";

// ============================================================
// OpenGLRenderer implementation
// ============================================================

OpenGLRenderer::OpenGLRenderer()
    : m_ViewportWidth(800)
    , m_ViewportHeight(600)
{
}

OpenGLRenderer::~OpenGLRenderer()
{
    Shutdown();
}

bool OpenGLRenderer::Init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    m_Shader = std::make_unique<Shader>();
    if (!m_Shader->LoadFromSource(phongVertexShader, phongFragmentShader)) {
        std::cerr << "Failed to load Phong shader" << std::endl;
        return false;
    }

    m_WireframeShader = std::make_unique<Shader>();
    if (!m_WireframeShader->LoadFromSource(wireframeVertexShader, wireframeFragmentShader)) {
        std::cerr << "Failed to load wireframe shader" << std::endl;
        return false;
    }

    m_GridShader = std::make_unique<Shader>();
    if (!m_GridShader->LoadFromSource(lineVertexShader, lineFragmentShader)) {
        std::cerr << "Failed to load grid shader" << std::endl;
        return false;
    }

    // X/Y/Z for RenderAxes: Y always drawn here; X/Z only when grid is off (otherwise grid colors center lines).
    float axisLen = 50.0f;
    float axesData[] = {
        0.f, 0.f, 0.f,       1.f, 0.2f, 0.2f,
        axisLen, 0.f, 0.f,   1.f, 0.2f, 0.2f,
        0.f, 0.f, 0.f,       0.2f, 1.f, 0.2f,
        0.f, axisLen, 0.f,   0.2f, 1.f, 0.2f,
        0.f, 0.f, 0.f,       0.3f, 0.3f, 1.f,
        0.f, 0.f, axisLen,   0.3f, 0.3f, 1.f,
    };
    glGenVertexArrays(1, &m_AxesVAO);
    glGenBuffers(1, &m_AxesVBO);
    glBindVertexArray(m_AxesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_AxesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesData), axesData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    return true;
}

void OpenGLRenderer::Shutdown()
{
    m_Shader.reset();
    m_WireframeShader.reset();
    m_GridShader.reset();

    if (m_GridVAO) { glDeleteVertexArrays(1, &m_GridVAO); m_GridVAO = 0; }
    if (m_GridVBO) { glDeleteBuffers(1, &m_GridVBO); m_GridVBO = 0; }
    if (m_AxesVAO) { glDeleteVertexArrays(1, &m_AxesVAO); m_AxesVAO = 0; }
    if (m_AxesVBO) { glDeleteBuffers(1, &m_AxesVBO); m_AxesVBO = 0; }
}

void OpenGLRenderer::BeginFrame(const glm::vec4& clearColor)
{
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::EndFrame()
{
}

void OpenGLRenderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    glViewport(x, y, width, height);
    m_ViewportWidth = width;
    m_ViewportHeight = height;
}

// -------------------------------------------------------
// Grid helpers
// -------------------------------------------------------
void OpenGLRenderer::SetupGrid(float size, float spacing, bool colorNegativeAxes,
                               bool showAxisX, bool showAxisZ)
{
    if (m_GridVAO && size == m_LastGridSize && spacing == m_LastGridSpacing
        && colorNegativeAxes == m_LastGridColorNegativeAxes
        && showAxisX == m_LastShowAxisXOnGrid && showAxisZ == m_LastShowAxisZOnGrid) {
        return;
    }

    if (m_GridVAO) { glDeleteVertexArrays(1, &m_GridVAO); m_GridVAO = 0; }
    if (m_GridVBO) { glDeleteBuffers(1, &m_GridVBO); m_GridVBO = 0; }

    std::vector<float> verts;
    glm::vec3 gridColor(0.35f, 0.35f, 0.35f);
    glm::vec3 axisXColor(1.f, 0.2f, 0.2f);
    glm::vec3 axisZColor(0.3f, 0.3f, 1.f);

    int halfCount = static_cast<int>(size / spacing);
    for (int i = -halfCount; i <= halfCount; ++i) {
        float pos = i * spacing;

        if (i == 0 && !colorNegativeAxes) {
            glm::vec3 zPos = showAxisZ ? axisZColor : gridColor;
            glm::vec3 xPos = showAxisX ? axisXColor : gridColor;
            // Center Z at x=0: gray on −Z; +Z colored if showAxisZ
            verts.insert(verts.end(), {pos, 0.f, -size, gridColor.r, gridColor.g, gridColor.b});
            verts.insert(verts.end(), {pos, 0.f,     0.f, gridColor.r, gridColor.g, gridColor.b});
            verts.insert(verts.end(), {pos, 0.f,     0.f, zPos.r, zPos.g, zPos.b});
            verts.insert(verts.end(), {pos, 0.f,  size, zPos.r, zPos.g, zPos.b});
            // Center X at z=0: gray on −X; +X colored if showAxisX
            verts.insert(verts.end(), {-size, 0.f, pos, gridColor.r, gridColor.g, gridColor.b});
            verts.insert(verts.end(), {    0.f, 0.f, pos, gridColor.r, gridColor.g, gridColor.b});
            verts.insert(verts.end(), {    0.f, 0.f, pos, xPos.r, xPos.g, xPos.b});
            verts.insert(verts.end(), { size, 0.f, pos, xPos.r, xPos.g, xPos.b});
            continue;
        }

        // Line parallel to world Z at x = pos
        glm::vec3 cAlongZ = (i == 0) ? (showAxisZ ? axisZColor : gridColor) : gridColor;
        verts.insert(verts.end(), {pos, 0.f, -size, cAlongZ.r, cAlongZ.g, cAlongZ.b});
        verts.insert(verts.end(), {pos, 0.f,  size, cAlongZ.r, cAlongZ.g, cAlongZ.b});

        // Line parallel to world X at z = pos
        glm::vec3 cAlongX = (i == 0) ? (showAxisX ? axisXColor : gridColor) : gridColor;
        verts.insert(verts.end(), {-size, 0.f, pos, cAlongX.r, cAlongX.g, cAlongX.b});
        verts.insert(verts.end(), { size, 0.f, pos, cAlongX.r, cAlongX.g, cAlongX.b});
    }

    m_GridVertexCount = static_cast<uint32_t>(verts.size() / 6);
    m_LastGridSize = size;
    m_LastGridSpacing = spacing;
    m_LastGridColorNegativeAxes = colorNegativeAxes;
    m_LastShowAxisXOnGrid = showAxisX;
    m_LastShowAxisZOnGrid = showAxisZ;

    glGenVertexArrays(1, &m_GridVAO);
    glGenBuffers(1, &m_GridVBO);
    glBindVertexArray(m_GridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_GridVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

void OpenGLRenderer::RenderGrid(const glm::mat4& view, const glm::mat4& projection,
                                float size, float spacing, bool colorNegativeAxes,
                                bool showAxisX, bool showAxisZ)
{
    SetupGrid(size, spacing, colorNegativeAxes, showAxisX, showAxisZ);
    if (!m_GridVAO) return;

    m_GridShader->Use();
    m_GridShader->SetMat4("model", glm::mat4(1.0f));
    m_GridShader->SetMat4("view", view);
    m_GridShader->SetMat4("projection", projection);
    m_GridShader->SetFloat("alpha", 0.5f);

    glBindVertexArray(m_GridVAO);
    glDrawArrays(GL_LINES, 0, m_GridVertexCount);
    glBindVertexArray(0);
}

void OpenGLRenderer::RenderAxes(const glm::mat4& view, const glm::mat4& projection,
                                const RenderSettings& settings)
{
    if (!settings.showAxisX && !settings.showAxisY && !settings.showAxisZ)
        return;

    m_GridShader->Use();
    m_GridShader->SetMat4("model", glm::mat4(1.0f));
    m_GridShader->SetMat4("view", view);
    m_GridShader->SetMat4("projection", projection);
    m_GridShader->SetFloat("alpha", 0.9f);

    glLineWidth(2.0f);
    glBindVertexArray(m_AxesVAO);
    // X/Z on the grid plane: colored by grid when showGrid is on (avoids duplicate lines).
    if (settings.showAxisX && !settings.showGrid)
        glDrawArrays(GL_LINES, 0, 2);
    if (settings.showAxisY)
        glDrawArrays(GL_LINES, 2, 2);
    if (settings.showAxisZ && !settings.showGrid)
        glDrawArrays(GL_LINES, 4, 2);
    glBindVertexArray(0);
    glLineWidth(1.0f);
}

// -------------------------------------------------------
// Scene rendering
// -------------------------------------------------------
void OpenGLRenderer::RenderScene(std::shared_ptr<Scene> scene,
                                 std::shared_ptr<Camera> camera,
                                 const RenderSettings& settings)
{
    if (!scene || !camera) return;

    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix(
        static_cast<float>(m_ViewportWidth) / static_cast<float>(m_ViewportHeight));

    // --- Grid & Axes (render first, behind everything) ---
    if (settings.showGrid) {
        RenderGrid(view, projection, settings.gridSize, settings.gridSpacing,
                   settings.gridColorNegativeAxes, settings.showAxisX, settings.showAxisZ);
    }
    if (settings.showAxisX || settings.showAxisY || settings.showAxisZ) {
        RenderAxes(view, projection, settings);
    }

    // --- Phong pass ---
    bool doSolid = (settings.renderMode != RenderMode::Wireframe);
    bool doWire  = (settings.renderMode != RenderMode::Solid);

    if (doSolid) {
        m_Shader->Use();

        m_Shader->SetVec3("viewPos", camera->GetPosition());
        m_Shader->SetVec3("lightPos", settings.light.position);
        m_Shader->SetVec3("lightColor", settings.light.color);
        m_Shader->SetFloat("ambientStrength", settings.light.ambientStrength);
        m_Shader->SetFloat("diffuseStrength", settings.light.diffuseStrength);
        m_Shader->SetFloat("specularStrength", settings.light.specularStrength);

        m_Shader->SetMat4("view", view);
        m_Shader->SetMat4("projection", projection);

        for (const auto& entity : scene->GetEntities()) {
            if (!entity->IsVisible()) continue;
            auto model = entity->GetModel();
            if (!model) continue;

            glm::mat4 transform = entity->GetTransform();
            m_Shader->SetMat4("model", transform);

            for (auto& mesh : model->GetMeshes()) {
                if (mesh->material) {
                    m_Shader->SetVec3("material_ambient", mesh->material->GetAmbient());
                    m_Shader->SetVec3("material_diffuse", mesh->material->GetDiffuse());
                    m_Shader->SetVec3("material_specular", mesh->material->GetSpecular());
                    m_Shader->SetFloat("material_shininess", mesh->material->GetShininess());
                }

                if (mesh->VAO == 0) SetupMesh(mesh);
                RenderMesh(mesh, transform);
            }
        }
        m_Shader->Unbind();
    }

    // --- Wireframe overlay for selected or SolidWireframe mode ---
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.5f);

        m_WireframeShader->Use();
        m_WireframeShader->SetMat4("view", view);
        m_WireframeShader->SetMat4("projection", projection);

        for (const auto& entity : scene->GetEntities()) {
            if (!entity->IsVisible()) continue;
            auto model = entity->GetModel();
            if (!model) continue;

            bool drawWire = doWire || entity->IsSelected();
            if (!drawWire) continue;

            glm::mat4 transform = entity->GetTransform();
            m_WireframeShader->SetMat4("model", transform);

            glm::vec3 wc = entity->IsSelected()
                ? settings.wireColor
                : glm::vec3(0.6f, 0.6f, 0.6f);
            m_WireframeShader->SetVec3("wireframeColor", wc);

            for (auto& mesh : model->GetMeshes()) {
                if (mesh->VAO == 0) SetupMesh(mesh);
                RenderMesh(mesh, transform);
            }
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        m_WireframeShader->Unbind();
    }
}

void OpenGLRenderer::SetupMesh(std::shared_ptr<Mesh> mesh)
{
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Vertex),
                 mesh->vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(uint32_t),
                 mesh->indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

void OpenGLRenderer::RenderMesh(std::shared_ptr<Mesh> mesh, const glm::mat4& /*transform*/)
{
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void OpenGLRenderer::CleanupMesh(std::shared_ptr<Mesh> mesh)
{
    if (mesh->VAO) { glDeleteVertexArrays(1, &mesh->VAO); mesh->VAO = 0; }
    if (mesh->VBO) { glDeleteBuffers(1, &mesh->VBO); mesh->VBO = 0; }
    if (mesh->EBO) { glDeleteBuffers(1, &mesh->EBO); mesh->EBO = 0; }
}

// -------------------------------------------------------
// Ray picking (unchanged logic)
// -------------------------------------------------------
std::shared_ptr<Entity> OpenGLRenderer::PickEntity(std::shared_ptr<Scene> scene,
                                                   std::shared_ptr<Camera> camera,
                                                   int mouseX, int mouseY,
                                                   int screenWidth, int screenHeight)
{
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    glm::vec4 ray_clip(x, y, -1.0f, 1.0f);

    glm::mat4 projection = camera->GetProjectionMatrix(
        static_cast<float>(screenWidth) / static_cast<float>(screenHeight));
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::mat4 view = camera->GetViewMatrix();
    glm::vec3 ray_wor = glm::vec3(glm::inverse(view) * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    glm::vec3 rayOrigin = camera->GetPosition();
    glm::vec3 rayDir = ray_wor;

    std::shared_ptr<Entity> closestEntity = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (const auto& entity : scene->GetEntities()) {
        if (!entity->IsVisible()) continue;
        auto model = entity->GetModel();
        if (!model) continue;

        glm::mat4 transform = entity->GetTransform();

        for (auto& mesh : model->GetMeshes()) {
            for (size_t i = 0; i < mesh->indices.size(); i += 3) {
                glm::vec3 v0 = glm::vec3(transform * glm::vec4(mesh->vertices[mesh->indices[i]].position, 1.0f));
                glm::vec3 v1 = glm::vec3(transform * glm::vec4(mesh->vertices[mesh->indices[i + 1]].position, 1.0f));
                glm::vec3 v2 = glm::vec3(transform * glm::vec4(mesh->vertices[mesh->indices[i + 2]].position, 1.0f));

                float t;
                if (RayIntersectsTriangle(rayOrigin, rayDir, v0, v1, v2, t)) {
                    if (t < closestDistance) {
                        closestDistance = t;
                        closestEntity = entity;
                    }
                }
            }
        }
    }

    return closestEntity;
}

bool OpenGLRenderer::RayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                          const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                          float& t)
{
    const float EPSILON = 0.0000001f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDir, edge2);
    float a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON) return false;

    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDir, q);
    if (v < 0.0f || u + v > 1.0f) return false;

    t = f * glm::dot(edge2, q);
    return (t > EPSILON);
}

} // namespace Metaphysics
