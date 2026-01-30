#include "OpenGLRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace Metaphysics {

// Phong光照着色器
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

// 材质属性
uniform vec3 material_ambient;
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;

void main()
{
    // 增强的环境光，确保模型基础可见
    vec3 ambient = lightColor * material_ambient * 1.5;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * material_diffuse);
    
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material_shininess);
    vec3 specular = lightColor * (spec * material_specular);
    
    // 添加额外的填充光（从相机方向），避免背光面太暗
    vec3 fillLight = lightColor * material_diffuse * 0.3;
    
    vec3 result = ambient + diffuse + specular + fillLight;
    FragColor = vec4(result, 1.0);
}
)";

// 线框着色器
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
    // 初始化OpenGL设置
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 加载着色器
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

    return true;
}

void OpenGLRenderer::Shutdown()
{
    m_Shader.reset();
    m_WireframeShader.reset();
}

void OpenGLRenderer::BeginFrame(const glm::vec4& clearColor)
{
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::EndFrame()
{
    // 在这里可以添加后处理等操作
}

void OpenGLRenderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    glViewport(x, y, width, height);
    m_ViewportWidth = width;
    m_ViewportHeight = height;
}

void OpenGLRenderer::RenderScene(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera)
{
    if (!scene || !camera) return;

    m_Shader->Use();

    // 设置光照 - 光源位置相对于相机，确保模型总是被照亮
    glm::vec3 cameraPos = camera->GetPosition();
    glm::vec3 lightPos = cameraPos + glm::vec3(3.0f, 5.0f, 3.0f);  // 光源在相机右上方
    glm::vec3 lightColor(1.2f, 1.2f, 1.2f);  // 增强光照强度

    m_Shader->SetVec3("viewPos", cameraPos);
    m_Shader->SetVec3("lightPos", lightPos);
    m_Shader->SetVec3("lightColor", lightColor);

    // 设置视图和投影矩阵
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix((float)m_ViewportWidth / (float)m_ViewportHeight);
    
    m_Shader->SetMat4("view", view);
    m_Shader->SetMat4("projection", projection);

    // 渲染所有实体
    for (const auto& entity : scene->GetEntities()) {
        if (!entity->IsVisible()) continue;

        auto model = entity->GetModel();
        if (!model) continue;

        glm::mat4 transform = entity->GetTransform();
        m_Shader->SetMat4("model", transform);

        // 渲染模型的所有网格
        for (auto& mesh : model->GetMeshes()) {
            // 设置材质
            if (mesh->material) {
                m_Shader->SetVec3("material_ambient", mesh->material->GetAmbient());
                m_Shader->SetVec3("material_diffuse", mesh->material->GetDiffuse());
                m_Shader->SetVec3("material_specular", mesh->material->GetSpecular());
                m_Shader->SetFloat("material_shininess", mesh->material->GetShininess());
            }

            // 如果网格还没有设置OpenGL缓冲区，先设置
            if (mesh->VAO == 0) {
                SetupMesh(mesh);
            }

            RenderMesh(mesh, transform);
        }

        // 如果实体被选中，绘制线框
        if (entity->IsSelected()) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(2.0f);

            m_WireframeShader->Use();
            m_WireframeShader->SetMat4("model", transform);
            m_WireframeShader->SetMat4("view", view);
            m_WireframeShader->SetMat4("projection", projection);
            m_WireframeShader->SetVec3("wireframeColor", glm::vec3(1.0f, 0.5f, 0.0f));

            for (auto& mesh : model->GetMeshes()) {
                RenderMesh(mesh, transform);
            }

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            m_Shader->Use();
        }
    }

    m_Shader->Unbind();
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

    // 位置属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // 法线属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, normal));

    // 纹理坐标属性
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

void OpenGLRenderer::RenderMesh(std::shared_ptr<Mesh> mesh, const glm::mat4& transform)
{
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void OpenGLRenderer::CleanupMesh(std::shared_ptr<Mesh> mesh)
{
    if (mesh->VAO) {
        glDeleteVertexArrays(1, &mesh->VAO);
        mesh->VAO = 0;
    }
    if (mesh->VBO) {
        glDeleteBuffers(1, &mesh->VBO);
        mesh->VBO = 0;
    }
    if (mesh->EBO) {
        glDeleteBuffers(1, &mesh->EBO);
        mesh->EBO = 0;
    }
}

std::shared_ptr<Entity> OpenGLRenderer::PickEntity(std::shared_ptr<Scene> scene,
                                                   std::shared_ptr<Camera> camera,
                                                   int mouseX, int mouseY,
                                                   int screenWidth, int screenHeight)
{
    // 将屏幕坐标转换为NDC
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    float z = 1.0f;
    glm::vec3 ray_nds(x, y, z);

    // NDC -> Clip空间
    glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.0f, 1.0f);

    // Clip空间 -> Eye空间
    glm::mat4 projection = camera->GetProjectionMatrix((float)screenWidth / (float)screenHeight);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    // Eye空间 -> World空间
    glm::mat4 view = camera->GetViewMatrix();
    glm::vec3 ray_wor = glm::vec3(glm::inverse(view) * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    // 射线原点和方向
    glm::vec3 rayOrigin = camera->GetPosition();
    glm::vec3 rayDir = ray_wor;

    // 遍历所有实体，进行射线相交测试
    std::shared_ptr<Entity> closestEntity = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (const auto& entity : scene->GetEntities()) {
        if (!entity->IsVisible()) continue;

        auto model = entity->GetModel();
        if (!model) continue;

        glm::mat4 transform = entity->GetTransform();

        for (auto& mesh : model->GetMeshes()) {
            // 遍历所有三角形
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
    // Möller–Trumbore算法
    const float EPSILON = 0.0000001f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDir, edge2);
    float a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return false;

    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
    float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDir, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * glm::dot(edge2, q);

    if (t > EPSILON)
        return true;

    return false;
}

} // namespace Metaphysics
