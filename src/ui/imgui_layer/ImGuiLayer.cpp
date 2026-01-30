#include "ImGuiLayer.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace Metaphysics {

ImGuiLayer::ImGuiLayer()
{
    memset(m_ModelPathBuffer, 0, sizeof(m_ModelPathBuffer));
}

ImGuiLayer::~ImGuiLayer()
{
}

void ImGuiLayer::Init(void* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void ImGuiLayer::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::RenderUI(AppState& state)
{
    // 创建主Docking空间
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    RenderMenuBar(state);
    ImGui::End();

    // 渲染各个面板
    if (state.showSceneHierarchy) {
        RenderSceneHierarchy(state);
    }

    if (state.showProperties) {
        RenderProperties(state);
    }

    if (state.showModelLoader) {
        RenderModelLoader(state);
    }

    if (state.showDemoWindow) {
        ImGui::ShowDemoWindow(&state.showDemoWindow);
    }
}

void ImGuiLayer::RenderMenuBar(AppState& state)
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load Model", "Ctrl+O")) {
                state.showModelLoader = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // 这里可以触发退出事件
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Scene Hierarchy", nullptr, &state.showSceneHierarchy);
            ImGui::MenuItem("Properties", nullptr, &state.showProperties);
            ImGui::Separator();
            ImGui::MenuItem("Demo Window", nullptr, &state.showDemoWindow);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // 显示关于对话框
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void ImGuiLayer::RenderSceneHierarchy(AppState& state)
{
    ImGui::Begin("Scene Hierarchy", &state.showSceneHierarchy);

    if (!state.currentScene) {
        ImGui::Text("No scene loaded");
        ImGui::End();
        return;
    }

    ImGui::Text("Scene: %s", state.currentScene->GetName().c_str());
    ImGui::Separator();

    const auto& entities = state.currentScene->GetEntities();
    for (const auto& entity : entities) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        
        if (state.selectedEntity && state.selectedEntity->GetID() == entity->GetID()) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        ImGui::TreeNodeEx((void*)(intptr_t)entity->GetID(), flags, "%s", entity->GetName().c_str());

        if (ImGui::IsItemClicked()) {
            // 取消之前的选择
            if (state.selectedEntity) {
                state.selectedEntity->SetSelected(false);
            }
            
            state.selectedEntity = entity;
            state.selectedEntity->SetSelected(true);
            state.selectedMesh = nullptr; // 清除网格选择
        }

        // 右键菜单
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                state.currentScene->RemoveEntity(entity);
                if (state.selectedEntity == entity) {
                    state.selectedEntity = nullptr;
                }
            }
            if (ImGui::MenuItem("Duplicate")) {
                // 复制实体
                auto newEntity = state.currentScene->CreateEntity(
                    entity->GetName() + " (Copy)", 
                    entity->GetModel()
                );
                newEntity->SetPosition(entity->GetPosition());
                newEntity->SetRotation(entity->GetRotation());
                newEntity->SetScale(entity->GetScale());
            }
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

void ImGuiLayer::RenderProperties(AppState& state)
{
    ImGui::Begin("Properties", &state.showProperties);

    if (!state.selectedEntity) {
        ImGui::Text("No entity selected");
        ImGui::End();
        return;
    }

    auto entity = state.selectedEntity;

    // 名称
    char nameBuffer[256];
    strncpy(nameBuffer, entity->GetName().c_str(), sizeof(nameBuffer));
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        entity->SetName(nameBuffer);
    }

    ImGui::Separator();

    // Transform
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        glm::vec3 position = entity->GetPosition();
        if (ImGui::DragFloat3("Position", &position.x, 0.1f)) {
            entity->SetPosition(position);
        }

        glm::vec3 rotation = entity->GetRotation();
        if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f)) {
            entity->SetRotation(rotation);
        }

        glm::vec3 scale = entity->GetScale();
        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.001f, 100.0f)) {
            entity->SetScale(scale);
        }
    }

    ImGui::Separator();

    // 可见性
    bool visible = entity->IsVisible();
    if (ImGui::Checkbox("Visible", &visible)) {
        entity->SetVisible(visible);
    }

    ImGui::Separator();

    // 模型信息
    auto model = entity->GetModel();
    if (model && ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Name: %s", model->GetName().c_str());
        ImGui::Text("Path: %s", model->GetPath().c_str());
        ImGui::Text("Meshes: %zu", model->GetMeshes().size());

        ImGui::Spacing();
        ImGui::Text("Meshes:");
        
        int meshIndex = 0;
        for (auto& mesh : model->GetMeshes()) {
            char meshName[64];
            snprintf(meshName, sizeof(meshName), "Mesh %d", meshIndex++);
            
            if (ImGui::TreeNode(meshName)) {
                ImGui::Text("Vertices: %zu", mesh->vertices.size());
                ImGui::Text("Triangles: %zu", mesh->indices.size() / 3);

                // 材质编辑
                if (mesh->material && ImGui::TreeNode("Material")) {
                    glm::vec3 ambient = mesh->material->GetAmbient();
                    if (ImGui::ColorEdit3("Ambient", &ambient.x)) {
                        mesh->material->SetAmbient(ambient);
                    }

                    glm::vec3 diffuse = mesh->material->GetDiffuse();
                    if (ImGui::ColorEdit3("Diffuse", &diffuse.x)) {
                        mesh->material->SetDiffuse(diffuse);
                    }

                    glm::vec3 specular = mesh->material->GetSpecular();
                    if (ImGui::ColorEdit3("Specular", &specular.x)) {
                        mesh->material->SetSpecular(specular);
                    }

                    float shininess = mesh->material->GetShininess();
                    if (ImGui::SliderFloat("Shininess", &shininess, 1.0f, 256.0f)) {
                        mesh->material->SetShininess(shininess);
                    }

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }
        }
    }

    ImGui::End();
}

void ImGuiLayer::RenderModelLoader(AppState& state)
{
    ImGui::Begin("Load Model", &state.showModelLoader);

    ImGui::Text("Enter model file path:");
    ImGui::InputText("##ModelPath", m_ModelPathBuffer, sizeof(m_ModelPathBuffer));

    ImGui::Spacing();
    ImGui::Text("Supported formats: OBJ, FBX, GLTF, DAE, etc.");
    ImGui::Spacing();

    if (ImGui::Button("Load", ImVec2(120, 0))) {
        if (strlen(m_ModelPathBuffer) > 0 && m_OnModelLoad) {
            m_OnModelLoad(std::string(m_ModelPathBuffer));
            state.showModelLoader = false;
            memset(m_ModelPathBuffer, 0, sizeof(m_ModelPathBuffer));
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        state.showModelLoader = false;
        memset(m_ModelPathBuffer, 0, sizeof(m_ModelPathBuffer));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Recent Models:");
    ImGui::BulletText("Example: models/cube.obj");
    ImGui::BulletText("Example: models/sphere.fbx");

    ImGui::End();
}

} // namespace Metaphysics
