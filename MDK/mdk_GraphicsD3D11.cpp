#include "mdk_GraphicsD3D11.h"

namespace mdk
{

GfxBufferManagerD3D11::GfxBufferManagerD3D11 (Allocator& allocator)
    : manager_ (16u, allocator)
{
}

GfxBufferManagerD3D11::~GfxBufferManagerD3D11()
{
}

HGfxBuffer GfxBufferManagerD3D11::createVertexBuffer (size_t sizeInBytes, juce::uint32 flags, const void* initialData)
{
    return 0;
}

HGfxBuffer GfxBufferManagerD3D11::createIndexBuffer (size_t sizeInBytes, juce::uint32 flags, const void* initialData)
{
    return 0;
}

HGfxBuffer GfxBufferManagerD3D11::createConstantBuffer (size_t sizeInBytes)
{
    return 0;
}

bool GfxBufferManagerD3D11::updateBuffer (HGfxBuffer buffer, const void* data, size_t dataSize, bool dynamic)
{
    return false;
}

}   // namespace
