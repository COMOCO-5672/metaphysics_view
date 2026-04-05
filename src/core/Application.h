#pragma once

// GLEW must be included before any OpenGL headers
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../core/scene/Scene.h"
#include "../core/camera/Camera.h"
#include "../core/renderer/ISceneRenderer.h"
#include "../platform/opengl/OpenGLRenderer.h"
#include "../ui/imgui_layer/ImGuiLayer.h"
#ifdef TARGET_WINDOWS
#include "../platform/directx/DirectX11Renderer.h"
#endif

#include <memory>

namespace Metaphysics {

class Application {
public:
    Application();
    ~Application();

    bool Init();
    void Run();
    void Shutdown();

private:
    void ProcessInput();
    void Update(float deltaTime);
    void Render();

    void LoadModel(const std::string& path);

    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    GLFWwindow* m_Window = nullptr;

    std::unique_ptr<ISceneRenderer> m_Renderer;
    std::unique_ptr<ImGuiLayer> m_ImGuiLayer;

    AppState m_AppState;

    RendererAPIType m_ActiveAPI = RendererAPIType::OpenGL;

    float m_LastFrameTime = 0.0f;
    float m_DeltaTime = 0.0f;

    bool m_MousePressed = false;
    bool m_FirstMouse = true;
    double m_LastMouseX = 0.0;
    double m_LastMouseY = 0.0;

    static Application* s_Instance;
};

} // namespace Metaphysics
