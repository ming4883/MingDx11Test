#include "mdk_GraphicsD3D11.h"

namespace mdk
{

GfxBufferManagerD3D11::GfxBufferManagerD3D11 (Allocator& allocator)
    : soa_ (16u, allocator)
{
}

GfxBufferManagerD3D11::~GfxBufferManagerD3D11()
{
}

HGfxBuffer GfxBufferManagerD3D11::createVertexBuffer (size_t sizeInBytes, juce::uint32 flags, const void* initialData)
{
    bool dynamic = (flags & GfxBufferFlags::Dynamic) > 0;
    bool immutable = (flags & GfxBufferFlags::Immutable) > 0;

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = sizeInBytes;	// size
    desc.StructureByteStride = 0;   // for compute buffer only
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// vertex buffer
    desc.MiscFlags = 0;	// no misc flags

    if (dynamic)
    {
        desc.Usage = D3D11_USAGE_DYNAMIC;				// gpu read/write
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// cpu write only
    }
    else
    {
        desc.Usage = immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT;	// gpu read/write
        desc.CPUAccessFlags = 0;			                                    // no cpu sccess
    }

    D3D11_SUBRESOURCE_DATA srdara;
    srdara.pSysMem = initialData;

    ComObj<ID3D11Buffer> buffer;

    if (FAILED (apiDevice_->CreateBuffer (&desc, initialData == nullptr ? nullptr : &srdara, buffer)))
        return 0;

    SOA::Handle handle = soa_.acquire ();
    soa_.construct<Col> (handle);
    soa_.get<Col> (handle)->apiBuffer.set (buffer.drop(), false);

    return handle;
}

HGfxBuffer GfxBufferManagerD3D11::createIndexBuffer (size_t sizeInBytes, juce::uint32 flags, const void* initialData)
{
    return 0;
}

HGfxBuffer GfxBufferManagerD3D11::createConstantBuffer (size_t sizeInBytes)
{
    return 0;
}

void GfxBufferManagerD3D11::destroy (HGfxBuffer buffer)
{
}

bool GfxBufferManagerD3D11::updateBuffer (HGfxBuffer buffer, const void* data, size_t dataSize, bool dynamic)
{
    return false;
}

}   // namespace
