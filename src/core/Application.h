#pragma once

#include "../core/scene/Scene.h"
#include "../core/camera/Camera.h"
#include "../platform/opengl/OpenGLRenderer.h"
#include "../ui/imgui_layer/ImGuiLayer.h"
#include <GLFW/glfw3.h>
#include <memory>

namespace Metaphysics {

// 应用程序主类
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

    // GLFW回调
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    GLFWwindow* m_Window;
    
    std::unique_ptr<OpenGLRenderer> m_Renderer;
    std::unique_ptr<ImGuiLayer> m_ImGuiLayer;
    
    AppState m_AppState;
    
    // 时间
    float m_LastFrameTime;
    float m_DeltaTime;
    
    // 鼠标状态
    bool m_MousePressed;
    bool m_FirstMouse;
    double m_LastMouseX;
    double m_LastMouseY;
    
    // 静态实例（用于GLFW回调）
    static Application* s_Instance;
};

} // namespace Metaphysics
