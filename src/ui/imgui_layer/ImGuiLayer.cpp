#include "ImGuiLayer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <cstring>

#ifdef TARGET_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <commdlg.h>
#endif

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
#ifdef IMGUI_HAS_DOCKING
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

    ImGui::StyleColorsDark();

    // Slightly warmer, more readable style
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);

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
#ifdef IMGUI_HAS_DOCKING
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    RenderMenuBar(state);
    ImGui::End();
#else
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("MainMenuBar", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    RenderMenuBar(state);
    ImGui::End();
#endif

    if (state.showSceneHierarchy) RenderSceneHierarchy(state);
    if (state.showProperties)     RenderProperties(state);
    if (state.showRenderSettings) RenderRenderSettings(state);
    if (state.showModelLoader)    RenderModelLoader(state);
    if (state.showDemoWindow)     ImGui::ShowDemoWindow(&state.showDemoWindow);
}

// -------------------------------------------------------
void ImGuiLayer::RenderMenuBar(AppState& state)
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load Model", "Ctrl+O")) {
                state.showModelLoader = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Scene Hierarchy", nullptr, &state.showSceneHierarchy);
            ImGui::MenuItem("Properties",      nullptr, &state.showProperties);
            ImGui::MenuItem("Render Settings", nullptr, &state.showRenderSettings);
            ImGui::Separator();
            ImGui::MenuItem("Demo Window",     nullptr, &state.showDemoWindow);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {}
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

// -------------------------------------------------------
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

        ImGui::TreeNodeEx((void*)(intptr_t)entity->GetID(), flags, "%s",
                          entity->GetName().c_str());

        if (ImGui::IsItemClicked()) {
            if (state.selectedEntity) state.selectedEntity->SetSelected(false);
            state.selectedEntity = entity;
            state.selectedEntity->SetSelected(true);
            state.selectedMesh = nullptr;
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                state.currentScene->RemoveEntity(entity);
                if (state.selectedEntity == entity) state.selectedEntity = nullptr;
            }
            if (ImGui::MenuItem("Duplicate")) {
                auto ne = state.currentScene->CreateEntity(
                    entity->GetName() + " (Copy)", entity->GetModel());
                ne->SetPosition(entity->GetPosition());
                ne->SetRotation(entity->GetRotation());
                ne->SetScale(entity->GetScale());
            }
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

// -------------------------------------------------------
void ImGuiLayer::RenderProperties(AppState& state)
{
    ImGui::Begin("Properties", &state.showProperties);

    if (!state.selectedEntity) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No entity selected");
        ImGui::End();
        return;
    }

    auto entity = state.selectedEntity;

    char nameBuffer[256];
    strncpy_s(nameBuffer, entity->GetName().c_str(), sizeof(nameBuffer) - 1);
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        entity->SetName(nameBuffer);
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        glm::vec3 position = entity->GetPosition();
        if (ImGui::DragFloat3("Position", &position.x, 0.1f))
            entity->SetPosition(position);

        glm::vec3 rotation = entity->GetRotation();
        if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f))
            entity->SetRotation(rotation);

        glm::vec3 scale = entity->GetScale();
        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.001f, 100.0f))
            entity->SetScale(scale);
    }

    ImGui::Separator();

    bool visible = entity->IsVisible();
    if (ImGui::Checkbox("Visible", &visible))
        entity->SetVisible(visible);

    ImGui::Separator();

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

                if (mesh->material && ImGui::TreeNode("Material")) {
                    glm::vec3 ambient = mesh->material->GetAmbient();
                    if (ImGui::ColorEdit3("Ambient", &ambient.x))
                        mesh->material->SetAmbient(ambient);

                    glm::vec3 diffuse = mesh->material->GetDiffuse();
                    if (ImGui::ColorEdit3("Diffuse", &diffuse.x))
                        mesh->material->SetDiffuse(diffuse);

                    glm::vec3 specular = mesh->material->GetSpecular();
                    if (ImGui::ColorEdit3("Specular", &specular.x))
                        mesh->material->SetSpecular(specular);

                    float shininess = mesh->material->GetShininess();
                    if (ImGui::SliderFloat("Shininess", &shininess, 1.0f, 256.0f))
                        mesh->material->SetShininess(shininess);

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }
        }
    }

    ImGui::End();
}

// -------------------------------------------------------
// New: Render Settings panel (light, grid, render mode)
// -------------------------------------------------------
void ImGuiLayer::RenderRenderSettings(AppState& state)
{
    ImGui::Begin("Render Settings", &state.showRenderSettings);

    auto& rs = state.renderSettings;

    // --- Render mode ---
    if (ImGui::CollapsingHeader("Render Mode", ImGuiTreeNodeFlags_DefaultOpen)) {
        int mode = static_cast<int>(rs.renderMode);
        ImGui::RadioButton("Solid",            &mode, 0); ImGui::SameLine();
        ImGui::RadioButton("Wireframe",        &mode, 1); ImGui::SameLine();
        ImGui::RadioButton("Solid+Wireframe",  &mode, 2);
        rs.renderMode = static_cast<RenderMode>(mode);
    }

    ImGui::Separator();

    // --- Light ---
    if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Position", &rs.light.position.x, 0.5f);
        ImGui::ColorEdit3("Color",    &rs.light.color.x);
        ImGui::SliderFloat("Ambient Strength",  &rs.light.ambientStrength,  0.0f, 1.0f);
        ImGui::SliderFloat("Diffuse Strength",  &rs.light.diffuseStrength,  0.0f, 2.0f);
        ImGui::SliderFloat("Specular Strength", &rs.light.specularStrength, 0.0f, 2.0f);
    }

    ImGui::Separator();

    // --- Background ---
    if (ImGui::CollapsingHeader("Background", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColorEdit3("Clear Color", &rs.clearColor.x);
    }

    ImGui::Separator();

    // --- Grid & Axes ---
    if (ImGui::CollapsingHeader("Helpers", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show Grid", &rs.showGrid);
        if (rs.showGrid) {
            ImGui::SliderFloat("Grid Size",    &rs.gridSize,    5.0f, 50.0f);
            ImGui::SliderFloat("Grid Spacing", &rs.gridSpacing, 0.5f, 5.0f);
        }
        ImGui::Checkbox("Show Axes", &rs.showAxes);
    }

    ImGui::Separator();

    // --- Camera info (read-only) ---
    if (state.camera && ImGui::CollapsingHeader("Camera Info")) {
        glm::vec3 pos = state.camera->GetPosition();
        ImGui::Text("Position: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
        glm::vec3 fwd = state.camera->GetFront();
        ImGui::Text("Forward:  (%.2f, %.2f, %.2f)", fwd.x, fwd.y, fwd.z);
        ImGui::Text("Zoom/FOV: %.1f", state.camera->GetZoom());
    }

    ImGui::End();
}

// -------------------------------------------------------
// Windows native file dialog helper
// -------------------------------------------------------
#ifdef TARGET_WINDOWS
static bool OpenFileDialog(char* outPath, size_t outPathSize)
{
    OPENFILENAMEA ofn;
    char szFile[512] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = GetActiveWindow();
    ofn.lpstrFile    = szFile;
    ofn.nMaxFile     = sizeof(szFile);
    ofn.lpstrFilter  = "Model Files (*.obj;*.fbx;*.gltf;*.glb;*.dae;*.stl)\0"
                       "*.obj;*.fbx;*.gltf;*.glb;*.dae;*.stl\0"
                       "All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle   = "Select a 3D Model";
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        strncpy_s(outPath, outPathSize, szFile, outPathSize - 1);
        return true;
    }
    return false;
}
#endif

// -------------------------------------------------------
void ImGuiLayer::RenderModelLoader(AppState& state)
{
    ImGui::Begin("Load Model", &state.showModelLoader);

    ImGui::Text("Model file path:");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 90);
    ImGui::InputText("##ModelPath", m_ModelPathBuffer, sizeof(m_ModelPathBuffer));

    ImGui::SameLine();
#ifdef TARGET_WINDOWS
    if (ImGui::Button("Browse...")) {
        if (OpenFileDialog(m_ModelPathBuffer, sizeof(m_ModelPathBuffer))) {
            // File selected — auto-load immediately
            if (m_OnModelLoad) {
                m_OnModelLoad(std::string(m_ModelPathBuffer));
                state.showModelLoader = false;
                memset(m_ModelPathBuffer, 0, sizeof(m_ModelPathBuffer));
            }
        }
    }
#else
    ImGui::TextDisabled("(no file dialog on this platform)");
#endif

    ImGui::Spacing();
    ImGui::Text("Supported: OBJ, FBX, GLTF, GLB, DAE, STL");
    ImGui::Spacing();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Quick Load:");
    if (ImGui::Button("models/cube.obj")) {
        if (m_OnModelLoad) {
            m_OnModelLoad("models/cube.obj");
            state.showModelLoader = false;
        }
    }

    ImGui::End();
}

} // namespace Metaphysics
