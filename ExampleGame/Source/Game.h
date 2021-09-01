#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "LizardEngine.h"
#include "CD3D12.h"

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

class GameApp : public LizardEngine::IApplication
{
public:
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
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12PipelineState> pipelineState;
    // 根签名
    // 根签名像一个箱子，，里面保存需要绑定到渲染流水线上的资源和着色器输入寄存器映射关系
    // 根签名有大小的限制（64DWORD）
    //
    // 根参数像占用箱子空间的物品
    // 每个根参数占用一定大小的根签名空间，只要不超过根签名的大小限制用户可以随意规划
    // @see《DirectX12 3D 游戏开发实战》7.6
    ComPtr<ID3D12RootSignature> rootSignature;
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
