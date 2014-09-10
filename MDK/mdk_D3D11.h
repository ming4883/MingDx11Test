#ifndef MDK_D3D11_H_INCLUDED
#define MDK_D3D11_H_INCLUDED

#include "mdk_Demo.h"

#include <dxgi.h>
#include <d3d11_1.h>

#pragma comment(lib, "D3D11.lib")

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

    void set (T* ptr)
    {
        if (ptr_)
            ptr_->Release();

        ptr_ = ptr;
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
    bool d3dStartup();
    void d3dShutdown();

    inline bool reportIfFailed (HRESULT hr, const char* errMsg)
    {
        return !reportIfFalse (SUCCEEDED (hr), errMsg);
    }
};

#define failed(x) reportIfFailed (x, #x)

struct D3D11Resource
{
	static ID3D11InputLayout* createInputLayout (
		ID3D11Device* d3dDevice,
		const D3D11_INPUT_ELEMENT_DESC* inputElements,
		const size_t inputElementCount,
		ID3DBlob* shaderByteCode);

	/*! Create a vertex buffer which allows:
		- gpu read / write
		- non dynamic usage (i.e. dynamic writing may be heavy)
		- cpu write only
	*/
	static ID3D11Buffer* createVertexBuffer (
		ID3D11Device* d3dDevice,
		size_t sizeInBytes,
		size_t stride,
		bool dynamic,
		const void* initialData = nullptr);

	/*! Create a index buffer which allows:
		- gpu read / write
		- non dynamic usage (i.e. dynamic writing may be heavy)
		- cpu write only
	*/
	static ID3D11Buffer* createIndexBuffer (
		ID3D11Device* d3dDevice,
		size_t sizeInBytes,
		size_t stride,
		bool dynamic,
		const void* initialData = nullptr);

	static ID3D11Buffer* createConstantBuffer (
		ID3D11Device* d3dDevice,
		size_t sizeInBytes);

	static ID3D11Buffer* createStructComputeBuffer (
		ID3D11Device* d3dDevice,
		size_t bufferSizeInBytes,
		size_t structSizeInBytes,
		const void* initialData = nullptr);

	static ID3D11Texture2D* createTexture2DRenderBuffer (
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		size_t mipLevels,
		DXGI_FORMAT dataFormat,
		DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);
	
	static ID3D11Texture2D* createTexture2DArrayRenderBuffer (
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		size_t arraySize,
		size_t mipLevels,
		DXGI_FORMAT dataFormat,
		DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);

	static ID3D11Texture2D* createTexture2DStagingBuffer (
		ID3D11Device* d3dDevice,
		size_t width,
		size_t height,
		size_t mipLevels,
		DXGI_FORMAT dataFormat);

	static ID3D11RenderTargetView* createRenderTargetView (
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		size_t mipLevel,
		DXGI_FORMAT rtvFormat = (DXGI_FORMAT)-1);
	
	static ID3D11DepthStencilView* createDepthStencilView (
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		size_t mipLevel,
		DXGI_FORMAT dsvFormat = (DXGI_FORMAT)-1);

	static ID3D11ShaderResourceView* createShaderResourceView (
		ID3D11Device* d3dDevice,
		ID3D11Texture2D* texture,
		DXGI_FORMAT srvFormat = (DXGI_FORMAT)-1);

	static ID3D11ShaderResourceView* createShaderResourceView (
		ID3D11Device* d3dDevice,
		ID3D11Buffer* buffer);
	
	static ID3D11UnorderedAccessView* createUnorderedAccessView (
		ID3D11Device* d3dDevice,
		ID3D11Buffer* buffer);
		
};

} // namespace

#endif	// MDK_D3D11_H_INCLUDED
