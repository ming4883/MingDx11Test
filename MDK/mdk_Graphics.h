#ifndef MDK_GRAPHICS_H_INCLUDED
#define MDK_GRAPHICS_H_INCLUDED

#include "mdk_Config.h"

namespace mdk
{

namespace GfxBufferTypes
{
    enum Value
    {
        Vertex,
        Index,
        Constant,
    };
}

namespace GfxBufferFlags
{
    enum Value
    {
        None = 0,
        Dynamic = 1 << 0,
        Immutable = 1 << 1,
    };
}

typedef uint32_t HGfxBuffer;

class GfxBufferManager
{
    m_noncopyable (GfxBufferManager)

public:
    GfxBufferManager() {}
    virtual ~GfxBufferManager() {}

    virtual HGfxBuffer createVertexBuffer (size_t sizeInBytes, juce::uint32 flags, const void* initialData = nullptr) = 0;
    virtual HGfxBuffer createIndexBuffer (size_t sizeInBytes, juce::uint32 flags, const void* initialData = nullptr) = 0;
    virtual HGfxBuffer createConstantBuffer (size_t sizeInBytes) = 0;

    virtual void destroy (HGfxBuffer buffer) = 0;
    
    virtual bool updateBuffer (HGfxBuffer buffer, const void* data, size_t dataSize, bool dynamic) = 0;
};


}

#endif // MDK_GRAPHICS_H_INCLUDED
