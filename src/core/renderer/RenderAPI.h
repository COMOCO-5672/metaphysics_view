#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

namespace Metaphysics {

// 渲染API抽象层 - 用于支持OpenGL/DirectX切换
enum class RendererAPIType {
    None = 0,
    OpenGL,
    DirectX11,
    DirectX12
};

// 顶点数据结构
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

// 渲染器抽象接口
class RenderAPI {
public:
    virtual ~RenderAPI() = default;

    virtual void Init() = 0;
    virtual void Clear(const glm::vec4& color) = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    
    virtual void DrawIndexed(uint32_t indexCount) = 0;
    virtual void DrawArrays(uint32_t vertexCount) = 0;

    static RendererAPIType GetAPI() { return s_API; }
    static void SetAPI(RendererAPIType api) { s_API = api; }

private:
    static RendererAPIType s_API;
};

} // namespace Metaphysics
