#ifndef MDK_D3D11_H_INCLUDED
#define MDK_D3D11_H_INCLUDED

#include "mdk_Demo.h"

#include <dxgi.h>
#include <d3d11_1.h>

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

    Hold& set (T* ptr)
    {
        if (ptr_)
            ptr_->Release();

        ptr_ = ptr;
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

class D3D11Demo : public Demo
{
public:
    D3D11Demo();
    virtual ~D3D11Demo();

    void paint (Graphics&) override;

protected:
    D3D_FEATURE_LEVEL d3dFeatureLevel_;
    Hold<ID3D11Device> d3dDevice_;
    Hold<ID3D11DeviceContext> d3dIMContext_;
    Hold<IDXGISwapChain1> d3dSwapchain_;
    Hold<ID3D11RenderTargetView> d3dBackBufRTView_;
    Hold<ID3D11SamplerState> d3dSampWrapLinear_;
    Hold<ID3D11SamplerState> d3dSampWrapPoint_;
    Hold<ID3D11SamplerState> d3dSampClampLinear_;
    Hold<ID3D11SamplerState> d3dSampClampPoint_;
    
protected:
    
    struct ShaderCompileDesc
    {
        const char* source; //!< the HLSL source code
        const char* name;   //!< a name for error messages
        const char* entry;  //!< entry function
        const char* target; //!< e.g. "fx_4_1", "vs_3_0"
        const char* secondaryData;

        D3D_SHADER_MACRO* defines;
        ID3DInclude* include;
        
        size_t sourceSize;  //!< if 0, set to strlen (source)
        size_t flags1;
        size_t flags2;      //!< ignored for non effect file
        size_t secondaryDataSize;
        size_t secondaryDataFlags;

        ShaderCompileDesc()
        {
            reset();
        }

        void reset();
    };

    struct ShaderCompileResult
    {
        ID3DBlob* byteCode;
        ID3DBlob* errorMessage;

        ShaderCompileResult (ID3DBlob* code, ID3DBlob* error)
            : byteCode (code)
            , errorMessage (error)
        {
        }
    };

    bool d3dStartup();
    void d3dShutdown();

    // render targets
    D3D11_VIEWPORT getViewport (float x_ratio, float y_ratio, float w_ratio, float h_ratio, float minz = 0.0f, float maxz = 1.0f);

    // shaders
#if 0
    ShaderCompileResult compileShader (const ShaderCompileDesc& desc);

    ID3DBlob* compileShaderFromBinaryData (const char* id, const char* entry, const char* target);
#endif

    ID3DBlob* loadShaderFromBinaryData (const char* id);

    ID3D11VertexShader* createVertexShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage = nullptr);

    ID3D11PixelShader* createPixelShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage = nullptr);

    ID3D11GeometryShader* createGeometryShader (ID3DBlob* bytecode, ID3D11ClassLinkage* linkage = nullptr);

    // buffers
    ID3D11InputLayout* createInputLayout (const D3D11_INPUT_ELEMENT_DESC* inputElements, const size_t inputElementCount, ID3DBlob* shaderByteCode);

    ID3D11InputLayout* createInputLayout (const Array<D3D11_INPUT_ELEMENT_DESC>& inputElements, ID3DBlob* shaderByteCode)
    {
        return createInputLayout (inputElements.begin(), (size_t)inputElements.size(), shaderByteCode);
    }

	ID3D11Buffer* createVertexBuffer (size_t sizeInBytes, size_t stride, bool dynamic, const void* initialData = nullptr);

    template<typename T>
    ID3D11Buffer* createVertexBuffer (size_t numOfElems, bool dynamic, const T* initialData = nullptr)
    {
        return createVertexBuffer (numOfElems * sizeof (T), sizeof (T), dynamic, initialData);
    }

    ID3D11Buffer* createIndexBuffer (size_t sizeInBytes, size_t stride, bool dynamic, const void* initialData = nullptr);

    template<typename T>
    ID3D11Buffer* createIndexBuffer (size_t numOfElems, bool dynamic, const T* initialData = nullptr)
    {
        return createIndexBuffer (numOfElems * sizeof (T), sizeof (T), dynamic, initialData);
    }

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

    // textures
    ID3D11Texture2D* createTexture2D (size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, const void* initialData = nullptr, size_t rowPitch = 0, size_t slicePitch = 0);

    ID3D11Texture2D* createTexture2DFromBinaryData (const char* id);

    ID3D11Texture2D* createTexture2DRT (size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);

    // views
    ID3D11RenderTargetView* createRenderTargetView (ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);
	
	ID3D11DepthStencilView* createDepthStencilView (ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT dsvFormat = (DXGI_FORMAT)-1);

	ID3D11ShaderResourceView* createShaderResourceView (ID3D11Texture2D* texture, DXGI_FORMAT srvFormat = (DXGI_FORMAT)-1);

	ID3D11ShaderResourceView* createShaderResourceView (ID3D11Buffer* buffer);

    // states
    ID3D11SamplerState* createSamplerState (const D3D11_SAMPLER_DESC& desc);
	
    // error reporting
    inline bool reportIfFailed (HRESULT hr, const char* errMsg)
    {
        return !reportIfFalse (SUCCEEDED (hr), errMsg);
    }
};

#define stringify(x) #x
#define tostr(x) stringify(x)
#define failed(x) reportIfFailed (x, #x "@" __FILE__ ":" tostr(__LINE__))


/*
    struct cb_align TestCB1
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

#define cb_align __declspec (align(4))
#define cb_nextrow __declspec (align(16))

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
