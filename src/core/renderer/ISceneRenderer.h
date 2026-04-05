#pragma once

#include "RenderAPI.h"
#include "RenderSettings.h"
#include "../scene/Scene.h"
#include "../camera/Camera.h"
#include <memory>

namespace Metaphysics {

struct UIRendererContext {
    RendererAPIType api = RendererAPIType::None;
    void* device = nullptr;
    void* deviceContext = nullptr;
};

class ISceneRenderer {
public:
    virtual ~ISceneRenderer() = default;

    virtual bool Init(void* window) = 0;
    virtual void Shutdown() = 0;

    virtual void BeginFrame(const glm::vec4& clearColor) = 0;
    virtual void EndFrame() = 0;

    virtual void RenderScene(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera,
                             const RenderSettings& settings) = 0;

    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

    virtual std::shared_ptr<Entity> PickEntity(std::shared_ptr<Scene> scene,
                                               std::shared_ptr<Camera> camera,
                                               int mouseX, int mouseY,
                                               int screenWidth, int screenHeight) = 0;

    virtual UIRendererContext GetUIContext() const = 0;
};

} // namespace Metaphysics
