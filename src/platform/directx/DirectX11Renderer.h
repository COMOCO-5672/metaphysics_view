#pragma once

#include "../../core/renderer/ISceneRenderer.h"

#ifdef TARGET_WINDOWS
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <unordered_map>
#include <vector>

namespace Metaphysics {

class DirectX11Renderer : public ISceneRenderer {
public:
    DirectX11Renderer();
    ~DirectX11Renderer();

    bool Init(void* window) override;
    void Shutdown() override;

    void BeginFrame(const glm::vec4& clearColor) override;
    void EndFrame() override;

    void RenderScene(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera,
                     const RenderSettings& settings) override;

    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

    std::shared_ptr<Entity> PickEntity(std::shared_ptr<Scene> scene,
                                       std::shared_ptr<Camera> camera,
                                       int mouseX, int mouseY,
                                       int screenWidth, int screenHeight) override;

    UIRendererContext GetUIContext() const override;

private:
    struct MeshGpuResources {
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
        uint32_t indexCount = 0;
    };

    struct LineVertex {
        glm::vec3 pos;
        glm::vec3 color;
    };

    struct SceneConstants {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 viewPos;
        glm::vec4 lightPos;
        glm::vec4 lightColor;
        glm::vec4 lightParams;
    };

    struct ObjectConstants {
        glm::mat4 model;
        glm::vec4 materialAmbient;
        glm::vec4 materialDiffuse;
        glm::vec4 materialSpecular;
        glm::vec4 materialParams;
    };

    struct LineConstants {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 alpha;
    };

private:
    bool CreateDeviceAndSwapChain(HWND hwnd);
    bool CreateRenderTargets();
    bool CreateShaders();
    bool CreateStates();
    bool CreateConstantBuffers();

    void RecreateSwapChainResources();

    void EnsureMeshResources(const std::shared_ptr<Mesh>& mesh);

    void BuildGrid(const RenderSettings& settings);
    void RenderGridAndAxes(const glm::mat4& view, const glm::mat4& projection,
                           const RenderSettings& settings);

    bool RayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                               const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                               float& t) const;

private:
    void* m_Window = nullptr;

    Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RTV;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencil;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DSV;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_MeshVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_MeshPS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_MeshLayout;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_LineVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_LinePS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_LineLayout;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_SceneCB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_ObjectCB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_LineCB;

    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RasterSolid;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RasterWire;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DepthState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_BlendState;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_GridVB;
    uint32_t m_GridVertexCount = 0;
    uint32_t m_GridVBSizeBytes = 0;

    std::unordered_map<const Mesh*, MeshGpuResources> m_MeshResources;

    uint32_t m_ViewportWidth = 800;
    uint32_t m_ViewportHeight = 600;
};

} // namespace Metaphysics
#endif
