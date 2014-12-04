#ifndef MDK_GRAPHICS_H_INCLUDED
#define MDK_GRAPHICS_H_INCLUDED

#include "mdk_Config.h"
#include "mdk_Allocator.h"

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

m_decl_handle (HGfxBuffer, uint32_t)

class Frontend;

class GfxService
{
    m_noncopyable (GfxService)

public:
    GfxService() {}
    virtual ~GfxService() {}

    virtual bool startup (Frontend& frontEnd) = 0;
    virtual void shutdown() = 0;

    virtual HGfxBuffer bufferCreateVertex (size_t sizeInBytes, uint32_t flags, const void* initialData = nullptr) = 0;
    virtual HGfxBuffer bufferCreateIndex (size_t sizeInBytes, uint32_t flags, const void* initialData = nullptr) = 0;
    virtual HGfxBuffer bufferCreateConstant (size_t sizeInBytes) = 0;

    virtual bool bufferDestroy (HGfxBuffer buffer) = 0;
    
    virtual bool bufferUpdate (HGfxBuffer buffer, const void* data, size_t dataSize, bool dynamic) = 0;
};

}

#endif // MDK_GRAPHICS_H_INCLUDED
