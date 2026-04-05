#include "Application.h"
#include "../core/model/Model.h"
#include "imgui.h"
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <iostream>

namespace Metaphysics {

Application* Application::s_Instance = nullptr;

Application::Application()
{
    s_Instance = this;
}

Application::~Application()
{
    Shutdown();
}

bool Application::Init()
{
#ifdef TARGET_WINDOWS
    if (const char* backend = std::getenv("METAPHYSICS_RENDERER")) {
        std::string value(backend);
        for (char& c : value) c = static_cast<char>(::tolower(c));
        if (value == "dx11" || value == "directx" || value == "directx11") {
            m_ActiveAPI = RendererAPIType::DirectX11;
        }
    }
#endif

    RenderAPI::SetAPI(m_ActiveAPI);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    if (m_ActiveAPI == RendererAPIType::OpenGL) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    } else {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    m_Window = glfwCreateWindow(1280, 720, "Metaphysics 3D Renderer", nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    if (m_ActiveAPI == RendererAPIType::OpenGL) {
        glfwMakeContextCurrent(m_Window);
        glfwSwapInterval(1);

        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return false;
        }

        m_Renderer = std::make_unique<OpenGLRenderer>();
    }
#ifdef TARGET_WINDOWS
    else if (m_ActiveAPI == RendererAPIType::DirectX11) {
        m_Renderer = std::make_unique<DirectX11Renderer>();
    }
#endif

    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
    glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_Window, CursorPosCallback);
    glfwSetScrollCallback(m_Window, ScrollCallback);
    glfwSetKeyCallback(m_Window, KeyCallback);

    if (!m_Renderer || !m_Renderer->Init(m_Window)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }

    m_ImGuiLayer = std::make_unique<ImGuiLayer>();
    const auto uiCtx = m_Renderer->GetUIContext();
    ImGuiInitInfo initInfo{};
    initInfo.window = m_Window;
    initInfo.rendererAPI = m_ActiveAPI;
    initInfo.device = uiCtx.device;
    initInfo.deviceContext = uiCtx.deviceContext;

    if (!m_ImGuiLayer->Init(initInfo)) {
        std::cerr << "Failed to initialize ImGui layer" << std::endl;
        return false;
    }

    m_ImGuiLayer->OnModelLoadRequest([this](const std::string& path) { LoadModel(path); });

    m_AppState.currentScene = std::make_shared<Scene>("Main Scene");
    m_AppState.camera = std::make_shared<Camera>(
        glm::vec3(5.0f, 5.0f, 5.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -135.0f, -30.0f);

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(m_Window, &width, &height);
    m_AppState.screenWidth = width;
    m_AppState.screenHeight = height;
    m_Renderer->SetViewport(0, 0, width, height);

    std::cout << "Application initialized with renderer: "
              << (m_ActiveAPI == RendererAPIType::DirectX11 ? "DirectX11" : "OpenGL")
              << std::endl;

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

void Application::Update(float /*deltaTime*/) {}

void Application::Render()
{
    m_Renderer->BeginFrame(m_AppState.renderSettings.clearColor);
    m_Renderer->RenderScene(m_AppState.currentScene, m_AppState.camera, m_AppState.renderSettings);

    m_ImGuiLayer->BeginFrame();
    m_ImGuiLayer->RenderUI(m_AppState);
    m_ImGuiLayer->EndFrame();

    m_Renderer->EndFrame();

    if (m_ActiveAPI == RendererAPIType::OpenGL) {
        glfwSwapBuffers(m_Window);
    }
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

        if (m_AppState.selectedEntity) {
            m_AppState.selectedEntity->SetSelected(false);
        }
        m_AppState.selectedEntity = entity;
        entity->SetSelected(true);
    } else {
        std::cerr << "Failed to load model: " << path << std::endl;
    }
}

void Application::FramebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    if (s_Instance && s_Instance->m_Renderer) {
        s_Instance->m_Renderer->SetViewport(0, 0, width, height);
        s_Instance->m_AppState.screenWidth = width;
        s_Instance->m_AppState.screenHeight = height;
    }
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/)
{
    if (!s_Instance) return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            s_Instance->m_MousePressed = true;
            s_Instance->m_FirstMouse = true;
            glfwGetCursorPos(window, &s_Instance->m_LastMouseX, &s_Instance->m_LastMouseY);
        } else if (action == GLFW_RELEASE) {
            double currentX = 0.0;
            double currentY = 0.0;
            glfwGetCursorPos(window, &currentX, &currentY);

            const double deltaX = currentX - s_Instance->m_LastMouseX;
            const double deltaY = currentY - s_Instance->m_LastMouseY;

            if (std::abs(deltaX) < 5.0 && std::abs(deltaY) < 5.0) {
                auto pickedEntity = s_Instance->m_Renderer->PickEntity(
                    s_Instance->m_AppState.currentScene,
                    s_Instance->m_AppState.camera,
                    static_cast<int>(currentX),
                    static_cast<int>(currentY),
                    s_Instance->m_AppState.screenWidth,
                    s_Instance->m_AppState.screenHeight);

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

void Application::CursorPosCallback(GLFWwindow* /*window*/, double xpos, double ypos)
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

    const float xoffset = static_cast<float>(xpos - s_Instance->m_LastMouseX);
    const float yoffset = static_cast<float>(s_Instance->m_LastMouseY - ypos);

    s_Instance->m_LastMouseX = xpos;
    s_Instance->m_LastMouseY = ypos;

    s_Instance->m_AppState.camera->ProcessMouseMovement(xoffset, yoffset);
}

void Application::ScrollCallback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset)
{
    if (!s_Instance) return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    s_Instance->m_AppState.camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

void Application::KeyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int mods)
{
    if (!s_Instance || action != GLFW_PRESS) return;

    if (key == GLFW_KEY_O && (mods & GLFW_MOD_CONTROL)) {
        s_Instance->m_AppState.showModelLoader = true;
    }

    if (key == GLFW_KEY_DELETE && s_Instance->m_AppState.selectedEntity) {
        s_Instance->m_AppState.currentScene->RemoveEntity(s_Instance->m_AppState.selectedEntity);
        s_Instance->m_AppState.selectedEntity = nullptr;
    }
}

} // namespace Metaphysics
