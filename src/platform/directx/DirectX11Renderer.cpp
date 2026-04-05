#include "DirectX11Renderer.h"

#ifdef TARGET_WINDOWS

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <d3dcompiler.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <limits>
#include <cstring>
#include <iterator>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace Metaphysics {
namespace {

static const char* kMeshShader = R"(
cbuffer SceneCB : register(b0)
{
    matrix view;
    matrix projection;
    float4 viewPos;
    float4 lightPos;
    float4 lightColor;
    float4 lightParams;
};

cbuffer ObjectCB : register(b1)
{
    matrix model;
    float4 materialAmbient;
    float4 materialDiffuse;
    float4 materialSpecular;
    float4 materialParams;
};

struct VSInput {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VSOutput {
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 wpos = mul(float4(input.pos, 1.0f), model);
    output.worldPos = wpos.xyz;

    float3x3 nmat = (float3x3)transpose(inverse(model));
    output.normal = mul(input.normal, nmat);

    output.pos = mul(wpos, view);
    output.pos = mul(output.pos, projection);
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    float3 norm = normalize(input.normal);
    float3 ldir = normalize(lightPos.xyz - input.worldPos);
    float3 vdir = normalize(viewPos.xyz - input.worldPos);
    float3 hdir = normalize(ldir + vdir);

    float3 ambient = lightParams.x * lightColor.xyz * materialAmbient.xyz;
    float diff = max(dot(norm, ldir), 0.0f);
    float3 diffuse = lightParams.y * lightColor.xyz * (diff * materialDiffuse.xyz);

    float specExp = max(materialParams.x, 1.0f);
    float spec = pow(max(dot(norm, hdir), 0.0f), specExp);
    float3 specular = lightParams.z * lightColor.xyz * (spec * materialSpecular.xyz);

    return float4(ambient + diffuse + specular, 1.0f);
}
)";

static const char* kLineShader = R"(
cbuffer LineCB : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 alpha;
};

struct VSInput {
    float3 pos : POSITION;
    float3 color : COLOR0;
};

struct VSOutput {
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 p = mul(float4(input.pos, 1.0f), model);
    p = mul(p, view);
    p = mul(p, projection);
    output.pos = p;
    output.color = input.color;
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return float4(input.color, alpha.x);
}
)";

bool CompileShader(const char* source, const char* entry, const char* target,
                   Microsoft::WRL::ComPtr<ID3DBlob>& outBlob)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> errBlob;
    HRESULT hr = D3DCompile(source, strlen(source), nullptr, nullptr, nullptr,
                            entry, target, flags, 0, outBlob.GetAddressOf(), errBlob.GetAddressOf());
    if (FAILED(hr)) {
        if (errBlob) {
            std::cerr << "D3DCompile failed: " << (const char*)errBlob->GetBufferPointer() << std::endl;
        }
        return false;
    }
    return true;
}

glm::mat4 ToDxProjection(const glm::mat4& glProjection)
{
    glm::mat4 clip(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.0f, 0.0f, 0.5f, 1.0f);
    return clip * glProjection;
}

} // namespace

DirectX11Renderer::DirectX11Renderer() = default;

DirectX11Renderer::~DirectX11Renderer()
{
    Shutdown();
}

bool DirectX11Renderer::Init(void* window)
{
    m_Window = window;
    HWND hwnd = glfwGetWin32Window(static_cast<GLFWwindow*>(window));
    if (!hwnd) {
        std::cerr << "Failed to get HWND from GLFW window" << std::endl;
        return false;
    }

    if (!CreateDeviceAndSwapChain(hwnd)) return false;
    if (!CreateRenderTargets()) return false;
    if (!CreateShaders()) return false;
    if (!CreateStates()) return false;
    if (!CreateConstantBuffers()) return false;

    SetViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
    return true;
}

void DirectX11Renderer::Shutdown()
{
    m_MeshResources.clear();
    m_GridVB.Reset();

    m_BlendState.Reset();
    m_DepthState.Reset();
    m_RasterWire.Reset();
    m_RasterSolid.Reset();

    m_LineCB.Reset();
    m_ObjectCB.Reset();
    m_SceneCB.Reset();

    m_LineLayout.Reset();
    m_LinePS.Reset();
    m_LineVS.Reset();
    m_MeshLayout.Reset();
    m_MeshPS.Reset();
    m_MeshVS.Reset();

    m_DSV.Reset();
    m_DepthStencil.Reset();
    m_RTV.Reset();

    if (m_Context) {
        m_Context->ClearState();
    }

    m_SwapChain.Reset();
    m_Context.Reset();
    m_Device.Reset();
}

void DirectX11Renderer::BeginFrame(const glm::vec4& clearColor)
{
    const float color[4] = {clearColor.r, clearColor.g, clearColor.b, clearColor.a};
    m_Context->ClearRenderTargetView(m_RTV.Get(), color);
    m_Context->ClearDepthStencilView(m_DSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    ID3D11RenderTargetView* rtvs[] = {m_RTV.Get()};
    m_Context->OMSetRenderTargets(1, rtvs, m_DSV.Get());

    const float blendFactor[4] = {0, 0, 0, 0};
    m_Context->OMSetBlendState(m_BlendState.Get(), blendFactor, 0xFFFFFFFF);
    m_Context->OMSetDepthStencilState(m_DepthState.Get(), 0);
}

void DirectX11Renderer::EndFrame()
{
    if (m_SwapChain) {
        m_SwapChain->Present(1, 0);
    }
}

void DirectX11Renderer::RenderScene(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera,
                                    const RenderSettings& settings)
{
    if (!scene || !camera) return;

    const glm::mat4 view = camera->GetViewMatrix();
    const glm::mat4 projGl = camera->GetProjectionMatrix(
        static_cast<float>(m_ViewportWidth) / static_cast<float>(m_ViewportHeight));
    const glm::mat4 projDx = ToDxProjection(projGl);

    SceneConstants sc{};
    sc.view = glm::transpose(view);
    sc.projection = glm::transpose(projDx);
    sc.viewPos = glm::vec4(camera->GetPosition(), 1.0f);
    sc.lightPos = glm::vec4(settings.light.position, 1.0f);
    sc.lightColor = glm::vec4(settings.light.color, 1.0f);
    sc.lightParams = glm::vec4(settings.light.ambientStrength,
                               settings.light.diffuseStrength,
                               settings.light.specularStrength,
                               0.0f);
    m_Context->UpdateSubresource(m_SceneCB.Get(), 0, nullptr, &sc, 0, 0);

    bool doSolid = (settings.renderMode != RenderMode::Wireframe);
    bool doWire = (settings.renderMode != RenderMode::Solid);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    m_Context->IASetInputLayout(m_MeshLayout.Get());
    m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_Context->VSSetShader(m_MeshVS.Get(), nullptr, 0);
    m_Context->PSSetShader(m_MeshPS.Get(), nullptr, 0);

    ID3D11Buffer* sceneCB[] = {m_SceneCB.Get()};
    ID3D11Buffer* objCB[] = {m_ObjectCB.Get()};
    m_Context->VSSetConstantBuffers(0, 1, sceneCB);
    m_Context->PSSetConstantBuffers(0, 1, sceneCB);
    m_Context->VSSetConstantBuffers(1, 1, objCB);
    m_Context->PSSetConstantBuffers(1, 1, objCB);

    if (doSolid) {
        m_Context->RSSetState(m_RasterSolid.Get());

        for (const auto& entity : scene->GetEntities()) {
            if (!entity->IsVisible() || !entity->GetModel()) continue;

            ObjectConstants oc{};
            oc.model = glm::transpose(entity->GetTransform());

            for (auto& mesh : entity->GetModel()->GetMeshes()) {
                if (mesh->material) {
                    oc.materialAmbient = glm::vec4(mesh->material->GetAmbient(), 1.0f);
                    oc.materialDiffuse = glm::vec4(mesh->material->GetDiffuse(), 1.0f);
                    oc.materialSpecular = glm::vec4(mesh->material->GetSpecular(), 1.0f);
                    oc.materialParams = glm::vec4(mesh->material->GetShininess(), 0, 0, 0);
                }

                m_Context->UpdateSubresource(m_ObjectCB.Get(), 0, nullptr, &oc, 0, 0);
                EnsureMeshResources(mesh);

                auto it = m_MeshResources.find(mesh.get());
                if (it == m_MeshResources.end()) continue;

                ID3D11Buffer* vb = it->second.vertexBuffer.Get();
                ID3D11Buffer* ib = it->second.indexBuffer.Get();
                m_Context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
                m_Context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
                m_Context->DrawIndexed(it->second.indexCount, 0, 0);
            }
        }
    }

    if (doWire) {
        m_Context->RSSetState(m_RasterWire.Get());

        for (const auto& entity : scene->GetEntities()) {
            if (!entity->IsVisible() || !entity->GetModel()) continue;

            ObjectConstants oc{};
            oc.model = glm::transpose(entity->GetTransform());
            const glm::vec3 color = entity->IsSelected() ? settings.wireColor : glm::vec3(0.6f);
            oc.materialAmbient = glm::vec4(color, 1.0f);
            oc.materialDiffuse = glm::vec4(color, 1.0f);
            oc.materialSpecular = glm::vec4(color, 1.0f);
            oc.materialParams = glm::vec4(8.0f, 0, 0, 0);

            m_Context->UpdateSubresource(m_ObjectCB.Get(), 0, nullptr, &oc, 0, 0);

            for (auto& mesh : entity->GetModel()->GetMeshes()) {
                EnsureMeshResources(mesh);
                auto it = m_MeshResources.find(mesh.get());
                if (it == m_MeshResources.end()) continue;

                ID3D11Buffer* vb = it->second.vertexBuffer.Get();
                ID3D11Buffer* ib = it->second.indexBuffer.Get();
                m_Context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
                m_Context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
                m_Context->DrawIndexed(it->second.indexCount, 0, 0);
            }
        }
    }

    m_Context->RSSetState(m_RasterSolid.Get());
    RenderGridAndAxes(view, projDx, settings);
}

void DirectX11Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    m_ViewportWidth = width;
    m_ViewportHeight = height;

    D3D11_VIEWPORT vp{};
    vp.TopLeftX = static_cast<float>(x);
    vp.TopLeftY = static_cast<float>(y);
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_Context->RSSetViewports(1, &vp);
}

std::shared_ptr<Entity> DirectX11Renderer::PickEntity(std::shared_ptr<Scene> scene,
                                                       std::shared_ptr<Camera> camera,
                                                       int mouseX, int mouseY,
                                                       int screenWidth, int screenHeight)
{
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    glm::mat4 projection = camera->GetProjectionMatrix(
        static_cast<float>(screenWidth) / static_cast<float>(screenHeight));
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::mat4 view = camera->GetViewMatrix();
    glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
    rayWorld = glm::normalize(rayWorld);

    glm::vec3 rayOrigin = camera->GetPosition();
    std::shared_ptr<Entity> closestEntity = nullptr;
    float closestDistance = (std::numeric_limits<float>::max)();

    for (const auto& entity : scene->GetEntities()) {
        if (!entity->IsVisible() || !entity->GetModel()) continue;

        glm::mat4 transform = entity->GetTransform();
        for (auto& mesh : entity->GetModel()->GetMeshes()) {
            for (size_t i = 0; i < mesh->indices.size(); i += 3) {
                glm::vec3 v0 = glm::vec3(transform * glm::vec4(mesh->vertices[mesh->indices[i]].position, 1.0f));
                glm::vec3 v1 = glm::vec3(transform * glm::vec4(mesh->vertices[mesh->indices[i + 1]].position, 1.0f));
                glm::vec3 v2 = glm::vec3(transform * glm::vec4(mesh->vertices[mesh->indices[i + 2]].position, 1.0f));

                float t;
                if (RayIntersectsTriangle(rayOrigin, rayWorld, v0, v1, v2, t) && t < closestDistance) {
                    closestDistance = t;
                    closestEntity = entity;
                }
            }
        }
    }

    return closestEntity;
}

UIRendererContext DirectX11Renderer::GetUIContext() const
{
    UIRendererContext ctx;
    ctx.api = RendererAPIType::DirectX11;
    ctx.device = m_Device.Get();
    ctx.deviceContext = m_Context.Get();
    return ctx;
}

bool DirectX11Renderer::CreateDeviceAndSwapChain(HWND hwnd)
{
    RECT rc{};
    GetClientRect(hwnd, &rc);
    m_ViewportWidth = static_cast<uint32_t>(rc.right - rc.left);
    m_ViewportHeight = static_cast<uint32_t>(rc.bottom - rc.top);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = m_ViewportWidth;
    sd.BufferDesc.Height = m_ViewportHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    D3D_FEATURE_LEVEL selected{};

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        levels,
        static_cast<UINT>(std::size(levels)),
        D3D11_SDK_VERSION,
        &sd,
        m_SwapChain.GetAddressOf(),
        m_Device.GetAddressOf(),
        &selected,
        m_Context.GetAddressOf());

    if (FAILED(hr)) {
        std::cerr << "D3D11CreateDeviceAndSwapChain failed: 0x" << std::hex << hr << std::dec << std::endl;
        return false;
    }
    return true;
}

bool DirectX11Renderer::CreateRenderTargets()
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (FAILED(hr)) return false;

    hr = m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_RTV.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_TEXTURE2D_DESC depthDesc{};
    depthDesc.Width = m_ViewportWidth;
    depthDesc.Height = m_ViewportHeight;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = m_Device->CreateTexture2D(&depthDesc, nullptr, m_DepthStencil.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = m_Device->CreateDepthStencilView(m_DepthStencil.Get(), nullptr, m_DSV.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;
}

bool DirectX11Renderer::CreateShaders()
{
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;

    if (!CompileShader(kMeshShader, "VSMain", "vs_5_0", vsBlob)) return false;
    if (!CompileShader(kMeshShader, "PSMain", "ps_5_0", psBlob)) return false;

    HRESULT hr = m_Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                               nullptr, m_MeshVS.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = m_Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
                                      nullptr, m_MeshPS.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_INPUT_ELEMENT_DESC meshLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = m_Device->CreateInputLayout(meshLayout, static_cast<UINT>(std::size(meshLayout)),
                                     vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                     m_MeshLayout.GetAddressOf());
    if (FAILED(hr)) return false;

    vsBlob.Reset();
    psBlob.Reset();

    if (!CompileShader(kLineShader, "VSMain", "vs_5_0", vsBlob)) return false;
    if (!CompileShader(kLineShader, "PSMain", "ps_5_0", psBlob)) return false;

    hr = m_Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                      nullptr, m_LineVS.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = m_Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
                                     nullptr, m_LinePS.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_INPUT_ELEMENT_DESC lineLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = m_Device->CreateInputLayout(lineLayout, static_cast<UINT>(std::size(lineLayout)),
                                     vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                     m_LineLayout.GetAddressOf());
    return SUCCEEDED(hr);
}

bool DirectX11Renderer::CreateStates()
{
    D3D11_RASTERIZER_DESC rs{};
    rs.FillMode = D3D11_FILL_SOLID;
    rs.CullMode = D3D11_CULL_NONE;
    rs.DepthClipEnable = TRUE;
    if (FAILED(m_Device->CreateRasterizerState(&rs, m_RasterSolid.GetAddressOf()))) return false;

    rs.FillMode = D3D11_FILL_WIREFRAME;
    if (FAILED(m_Device->CreateRasterizerState(&rs, m_RasterWire.GetAddressOf()))) return false;

    D3D11_DEPTH_STENCIL_DESC ds{};
    ds.DepthEnable = TRUE;
    ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    ds.DepthFunc = D3D11_COMPARISON_LESS;
    if (FAILED(m_Device->CreateDepthStencilState(&ds, m_DepthState.GetAddressOf()))) return false;

    D3D11_BLEND_DESC bd{};
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    return SUCCEEDED(m_Device->CreateBlendState(&bd, m_BlendState.GetAddressOf()));
}

bool DirectX11Renderer::CreateConstantBuffers()
{
    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    bd.ByteWidth = sizeof(SceneConstants);
    if (FAILED(m_Device->CreateBuffer(&bd, nullptr, m_SceneCB.GetAddressOf()))) return false;

    bd.ByteWidth = sizeof(ObjectConstants);
    if (FAILED(m_Device->CreateBuffer(&bd, nullptr, m_ObjectCB.GetAddressOf()))) return false;

    bd.ByteWidth = sizeof(LineConstants);
    return SUCCEEDED(m_Device->CreateBuffer(&bd, nullptr, m_LineCB.GetAddressOf()));
}

void DirectX11Renderer::EnsureMeshResources(const std::shared_ptr<Mesh>& mesh)
{
    if (!mesh) return;
    if (m_MeshResources.find(mesh.get()) != m_MeshResources.end()) return;

    MeshGpuResources gpu{};

    D3D11_BUFFER_DESC vbd{};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = static_cast<UINT>(mesh->vertices.size() * sizeof(Vertex));
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vinit{};
    vinit.pSysMem = mesh->vertices.data();

    if (FAILED(m_Device->CreateBuffer(&vbd, &vinit, gpu.vertexBuffer.GetAddressOf()))) return;

    D3D11_BUFFER_DESC ibd{};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = static_cast<UINT>(mesh->indices.size() * sizeof(uint32_t));
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA iinit{};
    iinit.pSysMem = mesh->indices.data();

    if (FAILED(m_Device->CreateBuffer(&ibd, &iinit, gpu.indexBuffer.GetAddressOf()))) return;

    gpu.indexCount = static_cast<uint32_t>(mesh->indices.size());
    m_MeshResources[mesh.get()] = gpu;
}

void DirectX11Renderer::BuildGrid(const RenderSettings& settings)
{
    std::vector<LineVertex> verts;
    glm::vec3 gridColor(0.35f, 0.35f, 0.35f);
    glm::vec3 axisX(1.0f, 0.2f, 0.2f);
    glm::vec3 axisY(0.2f, 1.0f, 0.2f);
    glm::vec3 axisZ(0.3f, 0.3f, 1.0f);

    if (settings.showGrid) {
        int halfCount = static_cast<int>(settings.gridSize / settings.gridSpacing);
        for (int i = -halfCount; i <= halfCount; ++i) {
            float pos = i * settings.gridSpacing;

            glm::vec3 cz = (i == 0 && settings.showAxisZ) ? axisZ : gridColor;
            glm::vec3 cx = (i == 0 && settings.showAxisX) ? axisX : gridColor;

            if (i == 0 && !settings.gridColorNegativeAxes) {
                verts.push_back({glm::vec3(pos, 0, -settings.gridSize), gridColor});
                verts.push_back({glm::vec3(pos, 0, 0), gridColor});
                verts.push_back({glm::vec3(pos, 0, 0), cz});
                verts.push_back({glm::vec3(pos, 0, settings.gridSize), cz});

                verts.push_back({glm::vec3(-settings.gridSize, 0, pos), gridColor});
                verts.push_back({glm::vec3(0, 0, pos), gridColor});
                verts.push_back({glm::vec3(0, 0, pos), cx});
                verts.push_back({glm::vec3(settings.gridSize, 0, pos), cx});
                continue;
            }

            verts.push_back({glm::vec3(pos, 0, -settings.gridSize), cz});
            verts.push_back({glm::vec3(pos, 0, settings.gridSize), cz});
            verts.push_back({glm::vec3(-settings.gridSize, 0, pos), cx});
            verts.push_back({glm::vec3(settings.gridSize, 0, pos), cx});
        }
    }

    float axisLen = settings.gridSize * 1.5f;
    if (settings.showAxisY) {
        verts.push_back({glm::vec3(0, 0, 0), axisY});
        verts.push_back({glm::vec3(0, axisLen, 0), axisY});
    }

    if (settings.showAxisX && !settings.showGrid) {
        verts.push_back({glm::vec3(0, 0, 0), axisX});
        verts.push_back({glm::vec3(axisLen, 0, 0), axisX});
    }

    if (settings.showAxisZ && !settings.showGrid) {
        verts.push_back({glm::vec3(0, 0, 0), axisZ});
        verts.push_back({glm::vec3(0, 0, axisLen), axisZ});
    }

    if (verts.empty()) {
        m_GridVB.Reset();
        m_GridVertexCount = 0;
        return;
    }

    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = static_cast<UINT>(verts.size() * sizeof(LineVertex));
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    const uint32_t requiredBytes = static_cast<uint32_t>(verts.size() * sizeof(LineVertex));
    if (!m_GridVB || requiredBytes > m_GridVBSizeBytes) {
        m_GridVB.Reset();
        if (FAILED(m_Device->CreateBuffer(&bd, nullptr, m_GridVB.GetAddressOf()))) {
            return;
        }
        m_GridVBSizeBytes = bd.ByteWidth;
    }

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = m_Context->Map(m_GridVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr)) {
        memcpy(mapped.pData, verts.data(), verts.size() * sizeof(LineVertex));
        m_Context->Unmap(m_GridVB.Get(), 0);
        m_GridVertexCount = static_cast<uint32_t>(verts.size());
    }
}

void DirectX11Renderer::RenderGridAndAxes(const glm::mat4& view, const glm::mat4& projection,
                                          const RenderSettings& settings)
{
    BuildGrid(settings);
    if (!m_GridVB || m_GridVertexCount == 0) return;

    UINT stride = sizeof(LineVertex);
    UINT offset = 0;

    m_Context->IASetInputLayout(m_LineLayout.Get());
    m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_Context->VSSetShader(m_LineVS.Get(), nullptr, 0);
    m_Context->PSSetShader(m_LinePS.Get(), nullptr, 0);

    LineConstants lc{};
    lc.model = glm::transpose(glm::mat4(1.0f));
    lc.view = glm::transpose(view);
    lc.projection = glm::transpose(projection);
    lc.alpha = glm::vec4(0.75f, 0, 0, 0);

    m_Context->UpdateSubresource(m_LineCB.Get(), 0, nullptr, &lc, 0, 0);
    ID3D11Buffer* lineCB[] = {m_LineCB.Get()};
    m_Context->VSSetConstantBuffers(0, 1, lineCB);
    m_Context->PSSetConstantBuffers(0, 1, lineCB);

    ID3D11Buffer* vb = m_GridVB.Get();
    m_Context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    m_Context->Draw(m_GridVertexCount, 0);
}

void DirectX11Renderer::RecreateSwapChainResources()
{
    if (!m_SwapChain) return;

    if (m_Context) {
        m_Context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    m_DSV.Reset();
    m_DepthStencil.Reset();
    m_RTV.Reset();

    m_SwapChain->ResizeBuffers(0, m_ViewportWidth, m_ViewportHeight, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTargets();
    SetViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
}

bool DirectX11Renderer::RayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                              const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                              float& t) const
{
    const float eps = 1e-7f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDir, edge2);
    float a = glm::dot(edge1, h);

    if (a > -eps && a < eps) return false;

    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDir, q);
    if (v < 0.0f || u + v > 1.0f) return false;

    t = f * glm::dot(edge2, q);
    return t > eps;
}

} // namespace Metaphysics
#endif
