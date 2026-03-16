#pragma once

#include "../../core/scene/Scene.h"
#include "../../core/camera/Camera.h"
#include "../../platform/opengl/OpenGLRenderer.h"
#include <memory>
#include <string>
#include <functional>

namespace Metaphysics {

struct AppState {
    std::shared_ptr<Scene> currentScene;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Entity> selectedEntity;
    std::shared_ptr<Mesh> selectedMesh;

    RenderSettings renderSettings;

    bool showDemoWindow = false;
    bool showSceneHierarchy = true;
    bool showProperties = true;
    bool showRenderSettings = true;
    bool showModelLoader = false;

    int screenWidth = 800;
    int screenHeight = 600;
};

class ImGuiLayer {
public:
    ImGuiLayer();
    ~ImGuiLayer();

    void Init(void* window);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void RenderUI(AppState& state);

    void OnModelLoadRequest(std::function<void(const std::string&)> callback) {
        m_OnModelLoad = callback;
    }

private:
    void RenderMenuBar(AppState& state);
    void RenderSceneHierarchy(AppState& state);
    void RenderProperties(AppState& state);
    void RenderRenderSettings(AppState& state);
    void RenderModelLoader(AppState& state);

private:
    std::function<void(const std::string&)> m_OnModelLoad;
    char m_ModelPathBuffer[512];
};

} // namespace Metaphysics
