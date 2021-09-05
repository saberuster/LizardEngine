module;

//#include "Windows.h"
#include "d3d12.h"
#include "wrl.h"
#include "dxgi1_4.h"
#include "D3Dcompiler.h"
#include "DirectXMath.h"
#include "DirectXPackedVector.h"
#include "DirectXColors.h"
#include "DirectXCollision.h"
#pragma warning(disable : 5050)

export module ExampleGame;

import D3D12Module;
import std.core;
import LizardEngine;
import LizardEngine.Windows;

using Microsoft::WRL::ComPtr;

class MathHelper
{
public:
    // Returns random float in [0, 1).
    static float RandF()
    {
        return (float)(rand()) / (float)RAND_MAX;
    }

    // Returns random float in [a, b).
    static float RandF(float a, float b)
    {
        return a + RandF() * (b - a);
    }

    static int Rand(int a, int b)
    {
        return a + rand() % ((b - a) + 1);
    }

    template <typename T>
    static T Min(const T &a, const T &b)
    {
        return a < b ? a : b;
    }

    template <typename T>
    static T Max(const T &a, const T &b)
    {
        return a > b ? a : b;
    }

    template <typename T>
    static T Lerp(const T &a, const T &b, float t)
    {
        return a + (b - a) * t;
    }

    template <typename T>
    static T Clamp(const T &x, const T &low, const T &high)
    {
        return x < low ? low : (x > high ? high : x);
    }

    // Returns the polar angle of the point (x,y) in [0, 2*PI).
    static float AngleFromXY(float x, float y);

    static DirectX::XMVECTOR SphericalToCartesian(float radius, float theta, float phi)
    {
        return DirectX::XMVectorSet(
            radius * sinf(phi) * cosf(theta),
            radius * cosf(phi),
            radius * sinf(phi) * sinf(theta),
            1.0f);
    }

    static DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX M)
    {
        // Inverse-transpose is just applied to normals.  So zero out
        // translation row so that it doesn't get into our inverse-transpose
        // calculation--we don't want the inverse-transpose of the translation.
        DirectX::XMMATRIX A = M;
        A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

        DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
        return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
    }

    static DirectX::XMFLOAT4X4 Identity4x4()
    {
        static DirectX::XMFLOAT4X4 I(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        return I;
    }

    static DirectX::XMVECTOR RandUnitVec3();
    static DirectX::XMVECTOR RandHemisphereUnitVec3(DirectX::XMVECTOR n);

    static const float Infinity;
    static const float Pi;
};

extern const int gNumFrameResources;

inline void d3dSetDebugName(IDXGIObject *obj, const char *name)
{
    if (obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}
inline void d3dSetDebugName(ID3D12Device *obj, const char *name)
{
    if (obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}
inline void d3dSetDebugName(ID3D12DeviceChild *obj, const char *name)
{
    if (obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}

inline std::wstring AnsiToWString(const std::string &str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

class d3dUtil
{
public:
    static bool IsKeyDown(int vkeyCode);

    static std::string ToString(HRESULT hr);

    static UINT CalcConstantBufferByteSize(UINT byteSize)
    {
        // Constant buffers must be a multiple of the minimum hardware
        // allocation size (usually 256 bytes).  So round up to nearest
        // multiple of 256.  We do this by adding 255 and then masking off
        // the lower 2 bytes which store all bits < 256.
        // Example: Suppose byteSize = 300.
        // (300 + 255) & ~255
        // 555 & ~255
        // 0x022B & ~0x00ff
        // 0x022B & 0xff00
        // 0x0200
        // 512
        return (byteSize + 255) & ~255;
    }

    static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring &filename);

    static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        ID3D12Device *device,
        ID3D12GraphicsCommandList *cmdList,
        const void *initData,
        UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource> &uploadBuffer);

    static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
        const std::wstring &filename,
        const D3D_SHADER_MACRO *defines,
        const std::string &entrypoint,
        const std::string &target);
};

class DxException
{
public:
    DxException() = default;
    DxException(HRESULT hr, const std::wstring &functionName, const std::wstring &filename, int lineNumber);

    std::wstring ToString() const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};

// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index
// buffers so that we can implement the technique described by Figure 6.3.
struct SubmeshGeometry
{
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    INT BaseVertexLocation = 0;

    // Bounding box of the geometry defined by this submesh.
    // This is used in later chapters of the book.
    DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
    // Give it a name so we can look it up by name.
    std::string Name;

    // System memory copies.  Use Blobs because the vertex/index format can be generic.
    // It is up to the client to cast appropriately.
    Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

    // Data about the buffers.
    UINT VertexByteStride = 0;
    UINT VertexBufferByteSize = 0;
    DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
    UINT IndexBufferByteSize = 0;

    // A MeshGeometry may store multiple geometries in one vertex/index buffer.
    // Use this container to define the Submesh geometries so we can draw
    // the Submeshes individually.
    std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
    {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
        vbv.StrideInBytes = VertexByteStride;
        vbv.SizeInBytes = VertexBufferByteSize;

        return vbv;
    }

    D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
    {
        D3D12_INDEX_BUFFER_VIEW ibv;
        ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
        ibv.Format = IndexFormat;
        ibv.SizeInBytes = IndexBufferByteSize;

        return ibv;
    }

    // We can free this memory after we finish upload to the GPU.
    void DisposeUploaders()
    {
        VertexBufferUploader = nullptr;
        IndexBufferUploader = nullptr;
    }
};

struct Light
{
    DirectX::XMFLOAT3 Strength = {0.5f, 0.5f, 0.5f};
    float FalloffStart = 1.0f;                         // point/spot light only
    DirectX::XMFLOAT3 Direction = {0.0f, -1.0f, 0.0f}; // directional/spot light only
    float FalloffEnd = 10.0f;                          // point/spot light only
    DirectX::XMFLOAT3 Position = {0.0f, 0.0f, 0.0f};   // point/spot light only
    float SpotPower = 64.0f;                           // spot light only
};

#define MaxLights 16

struct MaterialConstants
{
    DirectX::XMFLOAT4 DiffuseAlbedo = {1.0f, 1.0f, 1.0f, 1.0f};
    DirectX::XMFLOAT3 FresnelR0 = {0.01f, 0.01f, 0.01f};
    float Roughness = 0.25f;

    // Used in texture mapping.
    DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

// Simple struct to represent a material for our demos.  A production 3D engine
// would likely create a class hierarchy of Materials.
struct Material
{
    // Unique material name for lookup.
    std::string Name;

    // Index into constant buffer corresponding to this material.
    int MatCBIndex = -1;

    // Index into SRV heap for diffuse texture.
    int DiffuseSrvHeapIndex = -1;

    // Index into SRV heap for normal texture.
    int NormalSrvHeapIndex = -1;

    // Dirty flag indicating the material has changed and we need to update the constant buffer.
    // Because we have a material constant buffer for each FrameResource, we have to apply the
    // update to each FrameResource.  Thus, when we modify a material we should set
    // NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
    int NumFramesDirty = gNumFrameResources;

    // Material constant buffer data used for shading.
    DirectX::XMFLOAT4 DiffuseAlbedo = {1.0f, 1.0f, 1.0f, 1.0f};
    DirectX::XMFLOAT3 FresnelR0 = {0.01f, 0.01f, 0.01f};
    float Roughness = .25f;
    DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct Texture
{
    // Unique material name for lookup.
    std::string Name;

    std::wstring Filename;

    Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

#ifndef ReleaseCom
#define ReleaseCom(x)     \
    {                     \
        if (x)            \
        {                 \
            x->Release(); \
            x = nullptr;  \
        }                 \
    }
#endif
inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}
class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }

private:
    const HRESULT m_hr;
};

#define SAFE_RELEASE(p) \
    if (p)              \
    (p)->Release()

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}
using namespace DirectX;

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT4 color;
};

export class ExampleGame : public IApplication
{
public:
    ExampleGame(std::wstring &&inAssetsPath);

    virtual void Init() override;
    virtual void Tick() override;
    virtual void Quit() override;

    void InitPipeline();
    void LoadAssets();

    void WaitForPreviosFrame();

    std::wstring GetAssetFullPath(LPCWSTR assetName);

private:
    // Pipeline objects.
    ComPtr<ID3D12Device> device;

    // 视口
    CD3DX12_VIEWPORT viewport;
    // 裁剪矩形
    CD3DX12_RECT scissorRect;

    // 命令队列(commandQueue) 是 CPU 与 GPU 沟通的桥梁，为一个环形缓冲区(ring buffer)
    // 我们在 CPU 端将命令提交到队列中，GPU 从队列中拿到命令执行，从命令的提交到执行是一个异步操作
    ComPtr<ID3D12CommandQueue> commandQueue;
    // 我们通过 commandList 向 GPU 提交命令
    ComPtr<ID3D12GraphicsCommandList> commandList;
    // 我们提交的命令都会保存在 commandAllocator 中
    ComPtr<ID3D12CommandAllocator> commandAllocator;

    // 交换链
    // 一般都是 双缓冲(double buffering): 前台缓冲区(front buffer) 和 后台缓冲区(back buffer)
    // 前台缓冲区存储当前屏幕上显示的纹理, 后台缓冲区存储下一帧的画面, 然后通过交换前后缓冲区来显示下一帧内容, 这个操作叫做 presenting (在 API 中有对应的方法)
    // 据说可以为 三重缓冲 (triple buffering) 暂时还没研究
    // @see https://docs.microsoft.com/zh-cn/windows/win32/direct3d12/swap-chains
    // @see 《DirectX12 3D 游戏开发实战》4.1.4
    ComPtr<IDXGISwapChain3> swapChain;
    // 交换链的缓冲区数量, 我们使用双缓冲，所以这里是 2
    static constexpr UINT FrameCount = 2;
    ComPtr<ID3D12Resource> renderTargets[FrameCount];

    // Render Target View 渲染目标视图
    ComPtr<ID3D12DescriptorHeap> rtvHeap;

    // Shader resource view 着色器资源视图
    ComPtr<ID3D12DescriptorHeap> srvHeap;

    ComPtr<ID3D12PipelineState> pipelineState;
    // 根签名
    // 根签名像一个箱子，，里面保存需要绑定到渲染流水线上的资源和着色器输入寄存器映射关系
    // 根签名有大小的限制（64DWORD）
    //
    // 根参数像占用箱子空间的物品
    // 每个根参数占用一定大小的根签名空间，只要不超过根签名的大小限制用户可以随意规划
    // @see《DirectX12 3D 游戏开发实战》7.6
    ComPtr<ID3D12RootSignature> rootSignature;

    // 描述符大小
    // 因为 在不同的 GPU 平台描述符的大小不同，所以我们在初始化的时候需要手动获取记录一下
    // @see《DirectX12 3D 游戏开发实战》4.3.2
    UINT rtvDescriptorSize;

    // Synchronization objects.
    UINT frameIndex;
    HANDLE fenceEvent;
    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue;

    // Render Resources
    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    std::wstring assetsPath;
};

ExampleGame::ExampleGame(std::wstring &&inAssetsPath)
    : assetsPath(inAssetsPath),
      viewport(0.f, 0.f, 1920.f, 1080.f),
      scissorRect(0, 0, 1920.0l, 1080.0l),
      rtvDescriptorSize(0)
{
}

void ExampleGame::Init()
{
    InitPipeline();
    LoadAssets();
}

void ExampleGame::Tick()
{
    // commandAllocator 必须在内部所有命令执行过以后才能 Reset
    ThrowIfFailed(commandAllocator->Reset());

    // commandList 在 Reset 以后我们就可以向其中添加命令
    // 有点像事务的 begin 和 end，这里是 Reset 和 Close
    ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState.Get()));
    auto _t1 = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    // Indicate that the back buffer will be used as a render target.
    commandList->ResourceBarrier(1, &_t1);

    //---- Render Resource begin
    // Set necessary state.
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    auto _tt = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    // Indicate that the back buffer will be used as a render target.
    commandList->ResourceBarrier(1, &_tt);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->DrawInstanced(3, 1, 0, 0);

    //---- Render Resource end

    auto _t2 = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    // Indicate that the back buffer will now be used to present.
    commandList->ResourceBarrier(1, &_t2);

    // commandList 在调用 ExecuteCommandLists 之前必须先把它关闭，表示所有命令全部添加完成
    ThrowIfFailed(commandList->Close());

    // Execute the command list.
    ID3D12CommandList *ppCommandLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(swapChain->Present(1, 0));

    WaitForPreviosFrame();
}

void ExampleGame::Quit()
{
    WaitForPreviosFrame();
}

void ExampleGame::InitPipeline()
{

    UINT dxgiFactoryFlags = 0;

    ComPtr<ID3D12Debug> debugController;

    // 开启 debug 模式
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));

    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

    // 交换链初始化
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount; // 设置缓冲区的数量
    swapChainDesc.Width = 1920;
    swapChainDesc.Height = 1080;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> _swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        commandQueue.Get(), // Swap chain needs the queue so that it can force a flush on it.
        WindowsLauncher::GetHWND(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &_swapChain));

    ThrowIfFailed(factory->MakeWindowAssociation(WindowsLauncher::GetHWND(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(_swapChain.As(&swapChain));

    // 因为我们渲染时操作的是后台缓冲区，所以这里要获取当前后台缓冲区的 index
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

        rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
            device->CreateRenderTargetView(renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, rtvDescriptorSize);
        }
    }

    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
}

void ExampleGame::LoadAssets()
{
    // 创建新的根签名用来绑定流水线资源
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif
        Logger::Instance().PrintLog(L"find shader file: {}", GetAssetFullPath(L"Shader\\test.hlsl"));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"Shader\\test.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"Shader\\test.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
    }

    // Create the command list.
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    ThrowIfFailed(commandList->Close());

    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
            {
                {{0.0f, 0.25f * .5, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
                {{0.25f, -0.25f * .5, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
                {{-0.25f, -0.25f * .5, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}};

        const UINT vertexBufferSize = sizeof(triangleVertices);

        auto _p = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto _d = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

        // Note: using upload heaps to transfer static data like vert buffers is not
        // recommended. Every time the GPU needs it, the upload heap will be marshalled
        // over. Please read up on Default Heap usage. An upload heap is used here for
        // code simplicity and because there are very few verts to actually transfer.
        ThrowIfFailed(device->CreateCommittedResource(
            &_p,
            D3D12_HEAP_FLAG_NONE,
            &_d,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8 *pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = sizeof(Vertex);
        vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // Create synchronization objects.
    {
        ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        WaitForPreviosFrame();
    }
}

void ExampleGame::WaitForPreviosFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 _fence = fenceValue;
    ThrowIfFailed(commandQueue->Signal(fence.Get(), _fence));
    fenceValue++;

    // Wait until the previous frame is finished.
    if (fence->GetCompletedValue() < _fence)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(_fence, fenceEvent));
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    frameIndex = swapChain->GetCurrentBackBufferIndex();
}

std::wstring ExampleGame::GetAssetFullPath(LPCWSTR assetName)
{
    return assetsPath + assetName;
}