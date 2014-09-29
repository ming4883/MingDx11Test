#ifndef MDK_D3D11_H_INCLUDED
#define MDK_D3D11_H_INCLUDED

#include "mdk_Demo.h"

#pragma warning(disable:4324)

#include <dxgi.h>
#include <d3d11_1.h>

#define m_failed(x) reportFailed (x, __FILE__ "(" m_tostr(__LINE__) "): " #x)

/*
    struct TestCB1
    {
        XMFLOAT2 a; // 0x00
        XMFLOAT4 b; // 0x08
    };
    
    struct cb_align TestCB2
    {
        XMFLOAT2 a; // 0x00
        cb_nextrow
        XMFLOAT4 b; // 0x16
    };

    see http://msdn.microsoft.com/en-us/library/windows/desktop/bb509632(v=vs.85).aspx for details
*/
#define cbuffer struct
#define cbuffer_nextrow __declspec (align(16))

namespace mdk
{

/*! A small util for holding COM object-pointers
 */
template<class T>
class Hold
{
    T* ptr_;
    Hold& operator = (Hold&);
    Hold (const Hold&);

public:
    Hold (T *ptr = nullptr) : ptr_ (ptr)
    {
    }

    ~Hold()
    {
        if (ptr_)
            ptr_->Release();
    }

    Hold& set (T* ptr, bool addRef = false)
    {
        if (ptr_)
            ptr_->Release();

        ptr_ = ptr;

        if (addRef)
            ptr_->AddRef();
        return *this;
    }

    T* drop()
    {
        T* ret = ptr_;
        ptr_ = nullptr;
        return ret;
    }

    bool isNull() const
    {
        return nullptr == ptr_;
    }

    bool operator == (const T* raw) const
    {
        return ptr_ == raw;
    }

    bool operator != (const T* raw) const
    {
        return ptr_ != raw;
    }

    T* operator -> ()
    {
        return ptr_;
    }

    operator void** ()
    {
        return (void**)&ptr_;
    }

    operator T** ()
    {
        return &ptr_;
    }

    operator T* ()
    {
        return ptr_;
    }

    template<class U>
    HRESULT as(Hold<U>& ret)
    {
        U* u;
        HRESULT hr = ptr_->QueryInterface (__uuidof (U), (void**)&u);

        if (SUCCEEDED (hr))
            ret.set (u);

        return hr;
    }
};

class D3D11Context
{
    Demo* demo_;

public:
    D3D_FEATURE_LEVEL featureLevel;
    Hold<ID3D11Device> device;
    Hold<ID3D11DeviceContext> contextIM;
    Hold<IDXGISwapChain1> swapchain;
    Hold<ID3D11RenderTargetView> backBufRTView;
    Hold<ID3D11Texture2D> depthBuf;
    Hold<ID3D11DepthStencilView> depthBufDSView;
    Hold<ID3D11ShaderResourceView> depthBufSRView;

    Hold<ID3D11SamplerState> sampWrapLinear;
    Hold<ID3D11SamplerState> sampWrapPoint;
    Hold<ID3D11SamplerState> sampClampLinear;
    Hold<ID3D11SamplerState> sampClampPoint;
    
    Hold<ID3D11RasterizerState> rastCullNone;
    Hold<ID3D11RasterizerState> rastCullFront;
    Hold<ID3D11RasterizerState> rastCullBack;
    
    Hold<ID3D11DepthStencilState> depthTestOnWriteOn;
    Hold<ID3D11DepthStencilState> depthTestOnWriteOff;
    Hold<ID3D11DepthStencilState> depthTestOffWriteOn;
    Hold<ID3D11DepthStencilState> depthTestOffWriteOff;
    
public:
    D3D11Context (Demo* demo);
    ~D3D11Context();

    bool startup (void* hwnd);
    void shutdown();

    // shaders
    ID3DBlob* loadShaderFromAppData (const char* id);

    ID3D11VertexShader* createVertexShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage = nullptr);

    ID3D11PixelShader* createPixelShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage = nullptr);

    ID3D11GeometryShader* createGeometryShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage = nullptr);

    // buffers
    ID3D11InputLayout* createInputLayout (const D3D11_INPUT_ELEMENT_DESC* inputElements, const size_t inputElementCount, ID3DBlob* shaderByteCode);

    ID3D11InputLayout* createInputLayout (const Array<D3D11_INPUT_ELEMENT_DESC>& inputElements, ID3DBlob* shaderByteCode)
    {
        return createInputLayout (inputElements.begin(), (size_t)inputElements.size(), shaderByteCode);
    }

	ID3D11Buffer* createVertexBuffer (size_t sizeInBytes, bool dynamic, bool immutable, const void* initialData = nullptr);

    ID3D11Buffer* createIndexBuffer (size_t sizeInBytes, bool dynamic, bool immutable, const void* initialData = nullptr);

    ID3D11Buffer* createConstantBuffer (size_t sizeInBytes);

    template<typename T>
    ID3D11Buffer* createConstantBuffer()
    {
        return createConstantBuffer (sizeof (T));
    }

    bool updateBuffer (ID3D11Buffer* buffer, const void* data, size_t dataSize, bool dynamic);

    template<typename T>
    bool updateBuffer (ID3D11Buffer* buffer, const T& t, bool dynamic = true)
    {
        return updateBuffer (buffer, &t, sizeof (T), dynamic);
    }

    static bool updateBuffer (ID3D11DeviceContext* context, ID3D11Buffer* buffer, const void* data, size_t dataSize, bool dynamic);

    template<typename T>
    static bool updateBuffer (ID3D11DeviceContext* context, ID3D11Buffer* buffer, const T& t, bool dynamic = true)
    {
        return updateBuffer (context, buffer, &t, sizeof (T), dynamic);
    }

    // textures
    ID3D11Texture2D* createTexture2D (size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, const void* initialData = nullptr, size_t rowPitch = 0, size_t slicePitch = 0);

    ID3D11Texture2D* createTexture2DFromAppData (const char* id);

    ID3D11Texture2D* createTexture2DRT (size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);

    // views
    ID3D11RenderTargetView* createRenderTargetView (ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);
	
	ID3D11DepthStencilView* createDepthStencilView (ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT dsvFormat = (DXGI_FORMAT)-1);

	ID3D11ShaderResourceView* createShaderResourceView (ID3D11Texture2D* texture, DXGI_FORMAT srvFormat = (DXGI_FORMAT)-1);

	ID3D11ShaderResourceView* createShaderResourceView (ID3D11Buffer* buffer);

    // states
    ID3D11SamplerState* createSamplerState (const D3D11_SAMPLER_DESC& desc);

    ID3D11RasterizerState* createRasterizerState (const D3D11_RASTERIZER_DESC& desc);

    ID3D11DepthStencilState* createDepthStencilState (const D3D11_DEPTH_STENCIL_DESC& desc);
	
    // error reporting
    inline bool reportFailed (HRESULT hr, const char* errMsg)
    {
        return demo_->reportTrue (FAILED (hr), errMsg);
    }

    inline bool reportTrue (bool boolVal, const char* errMsg)
    {
        return demo_->reportTrue (boolVal, errMsg);
    }
};

class D3D11Demo : public Demo
{
public:
    D3D11Demo();
    virtual ~D3D11Demo();

    void paint (Graphics&) override;

protected:
    D3D11Context d3d11;
    
protected:
    // render targets
    D3D11_VIEWPORT getViewport (float x_ratio, float y_ratio, float w_ratio, float h_ratio, float minz = 0.0f, float maxz = 1.0f);
    float getAspect();
};

class D3D11Resource
{
public:
    static ID3D11Buffer* createStructComputeBuffer (ID3D11Device* d3dDevice, size_t bufferSizeInBytes, size_t structSizeInBytes, const void* initialData = nullptr);

	static ID3D11Texture2D* createTexture2DArrayRenderBuffer (ID3D11Device* d3dDevice, size_t width, size_t height, size_t arraySize, size_t mipLevels, DXGI_FORMAT dataFormat, DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);

	static ID3D11Texture2D* createTexture2DStagingBuffer (ID3D11Device* d3dDevice, size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat);

	static ID3D11UnorderedAccessView* createUnorderedAccessView (ID3D11Device* d3dDevice, ID3D11Buffer* buffer);
};

} // namespace

#endif	// MDK_D3D11_H_INCLUDED
