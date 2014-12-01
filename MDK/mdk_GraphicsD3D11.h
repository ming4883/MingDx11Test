#ifndef MDK_GRAPHICSD3D11_H_INCLUDED
#define MDK_GRAPHICSD3D11_H_INCLUDED

#include "mdk_Graphics.h"

#include "mdk_SOAManager.h"

#pragma warning(disable:4324)
#define NOMINMAX

#include <dxgi.h>
#include <d3d11_1.h>

namespace mdk
{

/*! A small util for holding COM object-pointers
 */
template<class T>
class ComObj
{
    //m_noncopyable (ComObj)

public:
    ComObj (T* ptr = nullptr) : ptr_ (ptr)
    {
    }

    ~ComObj()
    {
        if (ptr_)
            ptr_->Release();
    }

    ComObj& set (T* ptr, bool addRef = false)
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
    HRESULT as (ComObj<U>& ret)
    {
        U* u;
        HRESULT hr = ptr_->QueryInterface (__uuidof (U), (void**)&u);

        if (SUCCEEDED (hr))
            ret.set (u);

        return hr;
    }

private:
    T* ptr_;
};

class GfxBufferD3D11
{
public:
    ComObj<ID3D11Buffer> apiBuffer;
};

class GfxBufferManagerD3D11 : public GfxBufferManager
{
public:
    GfxBufferManagerD3D11 (Allocator& allocator);
    ~GfxBufferManagerD3D11();

    HGfxBuffer createVertexBuffer (size_t sizeInBytes, juce::uint32 flags, const void* initialData) override;
    HGfxBuffer createIndexBuffer (size_t sizeInBytes, juce::uint32 flags, const void* initialData) override;
    HGfxBuffer createConstantBuffer (size_t sizeInBytes) override;

    void destroy (HGfxBuffer buffer);

    bool updateBuffer (HGfxBuffer buffer, const void* data, size_t dataSize, bool dynamic) override;

private:
    typedef SOAManager<SOAManagerTraitsDefault<GfxBufferD3D11>> SOA;
    typedef SOAColumn<SOA, 0> Col;
    SOA soa_;
    ComObj<ID3D11Device> apiDevice_;
};



}

#endif // MDK_GRAPHICSD3D11_H_INCLUDED
