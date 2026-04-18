#include "ImGuiLayer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstring>
#include <cstdio>

#ifdef TARGET_WINDOWS
#include <imgui_impl_dx11.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <d3d11.h>
#endif

#include <GLFW/glfw3.h>

namespace Metaphysics {

ImGuiLayer::ImGuiLayer()
{
    memset(m_ModelPathBuffer, 0, sizeof(m_ModelPathBuffer));
}

ImGuiLayer::~ImGuiLayer() = default;

bool ImGuiLayer::Init(const ImGuiInitInfo& initInfo)
{
    m_RendererAPI = initInfo.rendererAPI;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO();

    ApplyBlenderStyle();

    GLFWwindow* window = static_cast<GLFWwindow*>(initInfo.window);
    if (!window) {
        return false;
    }

    if (m_RendererAPI == RendererAPIType::OpenGL) {
        if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) return false;
        if (!ImGui_ImplOpenGL3_Init("#version 330")) return false;
        return true;
    }

#ifdef TARGET_WINDOWS
    if (m_RendererAPI == RendererAPIType::DirectX11) {
        if (!ImGui_ImplGlfw_InitForOther(window, true)) return false;
        return ImGui_ImplDX11_Init(static_cast<ID3D11Device*>(initInfo.device),
                                   static_cast<ID3D11DeviceContext*>(initInfo.deviceContext));
    }
#endif

    return false;
}

void ImGuiLayer::Shutdown()
{
#ifdef TARGET_WINDOWS
    if (m_RendererAPI == RendererAPIType::DirectX11) {
        ImGui_ImplDX11_Shutdown();
    }
#endif

    if (m_RendererAPI == RendererAPIType::OpenGL) {
        ImGui_ImplOpenGL3_Shutdown();
    }

    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::BeginFrame()
{
#ifdef TARGET_WINDOWS
    if (m_RendererAPI == RendererAPIType::DirectX11) {
        ImGui_ImplDX11_NewFrame();
    }
#endif

    if (m_RendererAPI == RendererAPIType::OpenGL) {
        ImGui_ImplOpenGL3_NewFrame();
    }

    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::EndFrame()
{
    ImGui::Render();

#ifdef TARGET_WINDOWS
    if (m_RendererAPI == RendererAPIType::DirectX11) {
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        return;
    }
#endif

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::ApplyBlenderStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 2.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(7.0f, 5.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.145f, 0.149f, 0.157f, 1.0f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.118f, 0.122f, 0.129f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.165f, 0.173f, 0.184f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.198f, 0.212f, 0.235f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.122f, 0.129f, 0.137f, 0.98f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.216f, 0.227f, 0.243f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.267f, 0.290f, 0.322f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.294f, 0.318f, 0.353f, 1.0f);

    colors[ImGuiCol_Button] = ImVec4(0.243f, 0.267f, 0.302f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.310f, 0.349f, 0.400f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.361f, 0.420f, 0.490f, 1.0f);

    colors[ImGuiCol_Header] = ImVec4(0.235f, 0.255f, 0.282f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.286f, 0.318f, 0.365f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.333f, 0.376f, 0.443f, 1.0f);

    colors[ImGuiCol_Tab] = ImVec4(0.192f, 0.204f, 0.224f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.286f, 0.318f, 0.365f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.243f, 0.271f, 0.314f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.176f, 0.184f, 0.196f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.216f, 0.231f, 0.255f, 1.0f);

    colors[ImGuiCol_CheckMark] = ImVec4(0.400f, 0.620f, 0.980f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.396f, 0.569f, 0.867f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.459f, 0.663f, 1.0f, 1.0f);

    colors[ImGuiCol_Separator] = ImVec4(0.278f, 0.286f, 0.302f, 1.0f);
}

void ImGuiLayer::BuildBlenderDockLayout()
{
    // This repository uses the bundled upstream ImGui without docking extensions.
    // Keep the function as a no-op so the old ImGui path still compiles.
    m_DockLayoutBuilt = true;
}

void ImGuiLayer::RenderUI(AppState& state)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags hostFlags = ImGuiWindowFlags_MenuBar;
    hostFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    hostFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    hostFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    hostFlags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("BlenderLikeRoot", nullptr, hostFlags);
    ImGui::PopStyleVar(3);

    RenderTopMenuBar(state);

    if (!m_DockLayoutBuilt) {
        BuildBlenderDockLayout();
    }

    RenderStatusBar(state);

    ImGui::End();

    if (state.showSceneHierarchy) RenderSceneHierarchy(state);
    if (state.showProperties) RenderProperties(state);
    if (state.showRenderSettings) RenderRenderSettings(state);
    if (state.showModelLoader) RenderModelLoader(state);
    if (state.showDemoWindow) ImGui::ShowDemoWindow(&state.showDemoWindow);

    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("Viewport", nullptr,
                 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                 ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextUnformatted("3D Viewport");
    ImGui::Separator();
    ImGui::Text("Renderer: %s", m_RendererAPI == RendererAPIType::DirectX11 ? "DirectX11" : "OpenGL");
    ImGui::Text("Use mouse drag + WASDQE to navigate");
    if (state.selectedEntity) {
        ImGui::Text("Selected: %s", state.selectedEntity->GetName().c_str());
        ImGui::TextUnformatted("Press F to focus the selected object");
    } else {
        ImGui::TextUnformatted("No selection. Click an object to select it.");
    }
    ImGui::Text("Scene objects: %zu", state.currentScene ? state.currentScene->GetEntities().size() : 0);
    ImGui::End();
}

void ImGuiLayer::RenderTopMenuBar(AppState& state)
{
    if (!ImGui::BeginMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Load Model", "Ctrl+O")) {
            state.showModelLoader = true;
        }
        ImGui::Separator();
        ImGui::MenuItem("Exit", "Alt+F4");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Scene Hierarchy", nullptr, &state.showSceneHierarchy);
        ImGui::MenuItem("Properties", nullptr, &state.showProperties);
        ImGui::MenuItem("Render Settings", nullptr, &state.showRenderSettings);
        ImGui::Separator();
        ImGui::MenuItem("ImGui Demo", nullptr, &state.showDemoWindow);
        ImGui::EndMenu();
    }

    ImGui::SeparatorText("Workspace: Layout / Modeling / Rendering");
    ImGui::EndMenuBar();
}

void ImGuiLayer::RenderSceneHierarchy(AppState& state)
{
    ImGui::Begin("Scene Hierarchy", &state.showSceneHierarchy);

    if (!state.currentScene) {
        ImGui::TextUnformatted("No scene loaded");
        ImGui::End();
        return;
    }

    ImGui::Text("Scene: %s", state.currentScene->GetName().c_str());
    ImGui::Separator();

    for (const auto& entity : state.currentScene->GetEntities()) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (state.selectedEntity && state.selectedEntity->GetID() == entity->GetID()) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(entity->GetID())), flags,
                          "%s", entity->GetName().c_str());

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
                auto copy = state.currentScene->CreateEntity(entity->GetName() + " (Copy)", entity->GetModel());
                copy->SetPosition(entity->GetPosition());
                copy->SetRotation(entity->GetRotation());
                copy->SetScale(entity->GetScale());
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
        ImGui::TextDisabled("No entity selected");
        ImGui::End();
        return;
    }

    auto entity = state.selectedEntity;

    char nameBuffer[256];
    strncpy_s(nameBuffer, entity->GetName().c_str(), sizeof(nameBuffer) - 1);
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        entity->SetName(nameBuffer);
    }

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        glm::vec3 position = entity->GetPosition();
        glm::vec3 rotation = entity->GetRotation();
        glm::vec3 scale = entity->GetScale();

        if (ImGui::DragFloat3("Position", &position.x, 0.1f)) entity->SetPosition(position);
        if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f)) entity->SetRotation(rotation);
        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.001f, 100.0f)) entity->SetScale(scale);
    }

    bool visible = entity->IsVisible();
    if (ImGui::Checkbox("Visible", &visible)) {
        entity->SetVisible(visible);
    }

    auto model = entity->GetModel();
    if (model && ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Name: %s", model->GetName().c_str());
        ImGui::Text("Path: %s", model->GetPath().c_str());
        ImGui::Text("Meshes: %zu", model->GetMeshes().size());

        int meshIndex = 0;
        for (auto& mesh : model->GetMeshes()) {
            char meshName[64];
            snprintf(meshName, sizeof(meshName), "Mesh %d", meshIndex++);

            if (ImGui::TreeNode(meshName)) {
                ImGui::Text("Vertices: %zu", mesh->vertices.size());
                ImGui::Text("Triangles: %zu", mesh->indices.size() / 3);

                if (mesh->material && ImGui::TreeNode("Material")) {
                    glm::vec3 ambient = mesh->material->GetAmbient();
                    glm::vec3 diffuse = mesh->material->GetDiffuse();
                    glm::vec3 specular = mesh->material->GetSpecular();
                    float shininess = mesh->material->GetShininess();

                    if (ImGui::ColorEdit3("Ambient", &ambient.x)) mesh->material->SetAmbient(ambient);
                    if (ImGui::ColorEdit3("Diffuse", &diffuse.x)) mesh->material->SetDiffuse(diffuse);
                    if (ImGui::ColorEdit3("Specular", &specular.x)) mesh->material->SetSpecular(specular);
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

void ImGuiLayer::RenderRenderSettings(AppState& state)
{
    ImGui::Begin("Render Settings", &state.showRenderSettings);

    auto& rs = state.renderSettings;

    int mode = static_cast<int>(rs.renderMode);
    ImGui::RadioButton("Solid", &mode, static_cast<int>(RenderMode::Solid)); ImGui::SameLine();
    ImGui::RadioButton("Wireframe", &mode, static_cast<int>(RenderMode::Wireframe)); ImGui::SameLine();
    ImGui::RadioButton("Solid+Wire", &mode, static_cast<int>(RenderMode::SolidWireframe));
    rs.renderMode = static_cast<RenderMode>(mode);

    ImGui::Separator();
    ImGui::DragFloat3("Light Pos", &rs.light.position.x, 0.3f);
    ImGui::ColorEdit3("Light Color", &rs.light.color.x);
    ImGui::SliderFloat("Ambient", &rs.light.ambientStrength, 0.0f, 1.0f);
    ImGui::SliderFloat("Diffuse", &rs.light.diffuseStrength, 0.0f, 2.0f);
    ImGui::SliderFloat("Specular", &rs.light.specularStrength, 0.0f, 2.0f);

    ImGui::Separator();
    ImGui::ColorEdit3("Clear Color", &rs.clearColor.x);

    ImGui::Separator();
    ImGui::Checkbox("Show Grid", &rs.showGrid);
    if (rs.showGrid) {
        ImGui::SliderFloat("Grid Size", &rs.gridSize, 5.0f, 100.0f);
        ImGui::SliderFloat("Grid Spacing", &rs.gridSpacing, 0.2f, 5.0f);
        ImGui::Checkbox("Color negative axis", &rs.gridColorNegativeAxes);
    }

    ImGui::Checkbox("Axis X", &rs.showAxisX);
    ImGui::Checkbox("Axis Y", &rs.showAxisY);
    ImGui::Checkbox("Axis Z", &rs.showAxisZ);

    ImGui::End();
}

#ifdef TARGET_WINDOWS
static bool OpenFileDialog(char* outPath, size_t outPathSize)
{
    OPENFILENAMEA ofn;
    char szFile[512] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Model Files (*.obj;*.fbx;*.gltf;*.glb;*.dae;*.stl)\0"
                      "*.obj;*.fbx;*.gltf;*.glb;*.dae;*.stl\0"
                      "All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = "Select a 3D Model";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        strncpy_s(outPath, outPathSize, szFile, outPathSize - 1);
        return true;
    }
    return false;
}
#endif

void ImGuiLayer::RenderModelLoader(AppState& state)
{
    ImGui::Begin("Load Model", &state.showModelLoader);

    ImGui::TextUnformatted("Model file path:");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 90);
    ImGui::InputText("##ModelPath", m_ModelPathBuffer, sizeof(m_ModelPathBuffer));

    ImGui::SameLine();
#ifdef TARGET_WINDOWS
    if (ImGui::Button("Browse...")) {
        if (OpenFileDialog(m_ModelPathBuffer, sizeof(m_ModelPathBuffer)) && m_OnModelLoad) {
            m_OnModelLoad(std::string(m_ModelPathBuffer));
            state.showModelLoader = false;
            memset(m_ModelPathBuffer, 0, sizeof(m_ModelPathBuffer));
        }
    }
#else
    ImGui::TextDisabled("(No file dialog on this platform)");
#endif

    ImGui::Spacing();
    ImGui::TextUnformatted("Supported: OBJ, FBX, GLTF, GLB, DAE, STL");

    ImGui::Separator();
    if (ImGui::Button("Quick Load: models/cube.obj") && m_OnModelLoad) {
        m_OnModelLoad("models/cube.obj");
        state.showModelLoader = false;
    }
    if (ImGui::Button("Quick Load: Gundam Part") && m_OnModelLoad) {
        m_OnModelLoad("models/gundam_rx78/gundam_rx78_description/meshes/rx78_object_005-lib.dae");
        state.showModelLoader = false;
    }

    ImGui::End();
}

void ImGuiLayer::RenderStatusBar(AppState& state)
{
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 26.0f);
    ImGui::Separator();
    ImGui::BeginChild("##status_bar", ImVec2(0.0f, 24.0f), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    const char* selected = state.selectedEntity ? state.selectedEntity->GetName().c_str() : "None";
    ImGui::Text("Selected: %s", selected);
    ImGui::SameLine();
    ImGui::Text("| Scene Entities: %zu", state.currentScene ? state.currentScene->GetEntities().size() : 0);
    ImGui::SameLine();
    ImGui::Text("| Resolution: %dx%d", state.screenWidth, state.screenHeight);
    ImGui::SameLine();
    ImGui::Text("| Mode: %s",
                state.renderSettings.renderMode == RenderMode::Solid ? "Solid" :
                state.renderSettings.renderMode == RenderMode::Wireframe ? "Wireframe" : "Solid+Wire");

    ImGui::EndChild();
}

} // namespace Metaphysics
