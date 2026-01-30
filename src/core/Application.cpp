#include "Application.h"
#include "../core/model/Model.h"
#include "imgui.h"
#include <iostream>

namespace Metaphysics {

Application* Application::s_Instance = nullptr;

Application::Application()
    : m_Window(nullptr)
    , m_LastFrameTime(0.0f)
    , m_DeltaTime(0.0f)
    , m_MousePressed(false)
    , m_FirstMouse(true)
    , m_LastMouseX(0.0)
    , m_LastMouseY(0.0)
{
    s_Instance = this;
}

Application::~Application()
{
    Shutdown();
}

bool Application::Init()
{
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    m_Window = glfwCreateWindow(800, 600, "Metaphysics 3D Renderer", nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1); // VSync

    // 设置回调
    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
    glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_Window, CursorPosCallback);
    glfwSetScrollCallback(m_Window, ScrollCallback);
    glfwSetKeyCallback(m_Window, KeyCallback);

    // 初始化GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    // 初始化渲染器
    m_Renderer = std::make_unique<OpenGLRenderer>();
    if (!m_Renderer->Init()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }

    // 初始化ImGui
    m_ImGuiLayer = std::make_unique<ImGuiLayer>();
    m_ImGuiLayer->Init(m_Window);
    m_ImGuiLayer->OnModelLoadRequest([this](const std::string& path) {
        LoadModel(path);
    });

    // 创建默认场景和相机
    m_AppState.currentScene = std::make_shared<Scene>("Main Scene");
    m_AppState.camera = std::make_shared<Camera>(
        glm::vec3(5.0f, 5.0f, 5.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -135.0f, -30.0f
    );

    int width, height;
    glfwGetFramebufferSize(m_Window, &width, &height);
    m_AppState.screenWidth = width;
    m_AppState.screenHeight = height;
    m_Renderer->SetViewport(0, 0, width, height);

    std::cout << "Application initialized successfully!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  - W/A/S/D: Move camera" << std::endl;
    std::cout << "  - Q/E: Move camera up/down" << std::endl;
    std::cout << "  - Left Mouse + Drag: Rotate camera" << std::endl;
    std::cout << "  - Mouse Wheel: Zoom" << std::endl;
    std::cout << "  - Left Click (without drag): Pick object" << std::endl;
    std::cout << "  - Ctrl+O: Load model" << std::endl;

    return true;
}

void Application::Run()
{
    m_LastFrameTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(m_Window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        m_DeltaTime = currentFrame - m_LastFrameTime;
        m_LastFrameTime = currentFrame;

        ProcessInput();
        Update(m_DeltaTime);
        Render();

        glfwPollEvents();
    }
}

void Application::Shutdown()
{
    if (m_ImGuiLayer) {
        m_ImGuiLayer->Shutdown();
        m_ImGuiLayer.reset();
    }

    if (m_Renderer) {
        m_Renderer->Shutdown();
        m_Renderer.reset();
    }

    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }

    glfwTerminate();
}

void Application::ProcessInput()
{
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_Window, true);
    }

    // 相机移动
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS) {
        m_AppState.camera->ProcessKeyboard(CameraMovement::FORWARD, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS) {
        m_AppState.camera->ProcessKeyboard(CameraMovement::BACKWARD, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS) {
        m_AppState.camera->ProcessKeyboard(CameraMovement::LEFT, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS) {
        m_AppState.camera->ProcessKeyboard(CameraMovement::RIGHT, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS) {
        m_AppState.camera->ProcessKeyboard(CameraMovement::DOWN, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS) {
        m_AppState.camera->ProcessKeyboard(CameraMovement::UP, m_DeltaTime);
    }
}

void Application::Update(float deltaTime)
{
    // 这里可以添加游戏逻辑更新
}

void Application::Render()
{
    // 渲染3D场景
    m_Renderer->BeginFrame(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
    m_Renderer->RenderScene(m_AppState.currentScene, m_AppState.camera);
    m_Renderer->EndFrame();

    // 渲染ImGui
    m_ImGuiLayer->BeginFrame();
    m_ImGuiLayer->RenderUI(m_AppState);
    m_ImGuiLayer->EndFrame();

    glfwSwapBuffers(m_Window);
}

void Application::LoadModel(const std::string& path)
{
    std::cout << "Loading model: " << path << std::endl;

    auto model = std::make_shared<Model>();
    if (model->LoadFromFile(path)) {
        std::string entityName = model->GetName();
        auto entity = m_AppState.currentScene->CreateEntity(entityName, model);
        
        std::cout << "Model loaded successfully: " << entityName << std::endl;
        std::cout << "  Meshes: " << model->GetMeshes().size() << std::endl;
        
        // 选中新加载的实体
        if (m_AppState.selectedEntity) {
            m_AppState.selectedEntity->SetSelected(false);
        }
        m_AppState.selectedEntity = entity;
        entity->SetSelected(true);
    } else {
        std::cerr << "Failed to load model: " << path << std::endl;
    }
}

// GLFW回调实现
void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    if (s_Instance && s_Instance->m_Renderer) {
        s_Instance->m_Renderer->SetViewport(0, 0, width, height);
        s_Instance->m_AppState.screenWidth = width;
        s_Instance->m_AppState.screenHeight = height;
    }
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (!s_Instance) return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return; // ImGui正在使用鼠标

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            s_Instance->m_MousePressed = true;
            s_Instance->m_FirstMouse = true;
            glfwGetCursorPos(window, &s_Instance->m_LastMouseX, &s_Instance->m_LastMouseY);
        } else if (action == GLFW_RELEASE) {
            // 如果没有移动鼠标（点击而非拖拽），进行拾取
            double currentX, currentY;
            glfwGetCursorPos(window, &currentX, &currentY);
            
            double deltaX = currentX - s_Instance->m_LastMouseX;
            double deltaY = currentY - s_Instance->m_LastMouseY;
            
            if (std::abs(deltaX) < 5.0 && std::abs(deltaY) < 5.0) {
                // 执行拾取
                auto pickedEntity = s_Instance->m_Renderer->PickEntity(
                    s_Instance->m_AppState.currentScene,
                    s_Instance->m_AppState.camera,
                    static_cast<int>(currentX),
                    static_cast<int>(currentY),
                    s_Instance->m_AppState.screenWidth,
                    s_Instance->m_AppState.screenHeight
                );

                // 取消之前的选择
                if (s_Instance->m_AppState.selectedEntity) {
                    s_Instance->m_AppState.selectedEntity->SetSelected(false);
                }

                if (pickedEntity) {
                    s_Instance->m_AppState.selectedEntity = pickedEntity;
                    pickedEntity->SetSelected(true);
                    std::cout << "Picked entity: " << pickedEntity->GetName() << std::endl;
                } else {
                    s_Instance->m_AppState.selectedEntity = nullptr;
                    std::cout << "No entity picked" << std::endl;
                }
            }

            s_Instance->m_MousePressed = false;
        }
    }
}

void Application::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (!s_Instance || !s_Instance->m_MousePressed) return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    if (s_Instance->m_FirstMouse) {
        s_Instance->m_LastMouseX = xpos;
        s_Instance->m_LastMouseY = ypos;
        s_Instance->m_FirstMouse = false;
        return;
    }

    float xoffset = static_cast<float>(xpos - s_Instance->m_LastMouseX);
    float yoffset = static_cast<float>(s_Instance->m_LastMouseY - ypos);

    s_Instance->m_LastMouseX = xpos;
    s_Instance->m_LastMouseY = ypos;

    s_Instance->m_AppState.camera->ProcessMouseMovement(xoffset, yoffset);
}

void Application::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (!s_Instance) return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    s_Instance->m_AppState.camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (!s_Instance) return;

    if (action == GLFW_PRESS) {
        // Ctrl+O: 打开模型加载对话框
        if (key == GLFW_KEY_O && (mods & GLFW_MOD_CONTROL)) {
            s_Instance->m_AppState.showModelLoader = true;
        }
        
        // Delete: 删除选中的实体
        if (key == GLFW_KEY_DELETE && s_Instance->m_AppState.selectedEntity) {
            s_Instance->m_AppState.currentScene->RemoveEntity(s_Instance->m_AppState.selectedEntity);
            s_Instance->m_AppState.selectedEntity = nullptr;
        }
    }
}

} // namespace Metaphysics
