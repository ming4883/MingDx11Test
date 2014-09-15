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
    ShaderCompileResult compileShader (const ShaderCompileDesc& desc);

    ID3DBlob* compileShaderFromBinaryData (const char* id, const char* entry, const char* target);

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

    // error reporting
    inline bool reportIfFailed (HRESULT hr, const char* errMsg)
    {
        return !reportIfFalse (SUCCEEDED (hr), errMsg);
    }
};

#define failed(x) reportIfFailed (x, #x)

class D3D11Resource
{
public:
    static ID3D11Buffer* createStructComputeBuffer (ID3D11Device* d3dDevice, size_t bufferSizeInBytes, size_t structSizeInBytes, const void* initialData = nullptr);

	static ID3D11Texture2D* createTexture2DRenderBuffer (ID3D11Device* d3dDevice, size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat, DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);
	
	static ID3D11Texture2D* createTexture2DArrayRenderBuffer (ID3D11Device* d3dDevice, size_t width, size_t height, size_t arraySize, size_t mipLevels, DXGI_FORMAT dataFormat, DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);

	static ID3D11Texture2D* createTexture2DStagingBuffer (ID3D11Device* d3dDevice, size_t width, size_t height, size_t mipLevels, DXGI_FORMAT dataFormat);

	static ID3D11RenderTargetView* createRenderTargetView (ID3D11Device* d3dDevice, ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);
	
	static ID3D11DepthStencilView* createDepthStencilView (ID3D11Device* d3dDevice, ID3D11Texture2D* texture, size_t mipLevel, DXGI_FORMAT dsvFormat = (DXGI_FORMAT)-1);

	static ID3D11ShaderResourceView* createShaderResourceView (ID3D11Device* d3dDevice, ID3D11Texture2D* texture, DXGI_FORMAT srvFormat = (DXGI_FORMAT)-1);

	static ID3D11ShaderResourceView* createShaderResourceView (ID3D11Device* d3dDevice, ID3D11Buffer* buffer);
	
	static ID3D11UnorderedAccessView* createUnorderedAccessView (ID3D11Device* d3dDevice, ID3D11Buffer* buffer);
};

} // namespace

#endif	// MDK_D3D11_H_INCLUDED
