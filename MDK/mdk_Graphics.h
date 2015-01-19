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

namespace GfxCompareFunc
{
    enum Value
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
    };
}

namespace GfxStencilOperation
{
    enum Value
    {
        Keep,
        Zero,
        Replace,
        IncClamp,
        DecClamp,
        Invert,
        IncWrap,
        DecWrap,
    };
}

namespace GfxBlendOperation
{
    enum Value
    {
        Add,                // a + b
        Subtract,           // a - b
        ReverseSubtract,    // b - a
        Min,                // min (a, b)
        Max,                // max (a, b)
    };
}

namespace GfxBlendFactor
{
    enum Value
    {
        Zero,
        One,
        SourceColor,
        OneMinusSourceColor,
        SourceAlpha,
        OneMinusSourceAlpha,
        DestinationColor,
        OneMinusDestinationColor,
        DestinationAlpha,
        OneMinusDestinationAlpha,
        SourceAlphaSaturated,
        BlendColor,
        OneMinusBlendColor,
        BlendAlpha,
        OneMinusBlendAlpha,
    };
}

namespace GfxSamplerAddressMode
{
    enum Value
    {
        ClampToEdge,
        Repeat,
        MirrorRepeat,
        ClampToZero,
    };
}

namespace GfxSamplerFilterMode
{
    enum Value
    {
        Nearest,
        Linear,
    };
}

struct GfxStencilDesc
{
    GfxStencilOperation::Value stencilFailOp; // Stencil Test Failure Operation;
    GfxStencilOperation::Value depthFailOp; // Depth Test Failure Operation;
    GfxStencilOperation::Value depthStencilPassOp; // Depth Stencil Test Pass Operation;
    GfxCompareFunc::Value stencilCmpFunc;

    uint32_t readMask;
    uint32_t writeMask;
};

struct GfxDepthStencilDesc
{
    GfxStencilDesc stencilFrontFace;
    GfxStencilDesc stencilBackFace;
    GfxCompareFunc::Value depthCmpFunc;
    bool depthWriteEnabled;
};

struct GfxBlendDesc
{
    bool blendEnabled;
    GfxBlendOperation::Value rgbOp;
    GfxBlendOperation::Value alphaOp;

    GfxBlendFactor::Value rgbDstFactor;
    GfxBlendFactor::Value alphaDstFactor;

    GfxBlendFactor::Value rgbSrcFactor;
    GfxBlendFactor::Value alphaSrcFactor;
};

struct GfxSamplerDesc
{
    GfxSamplerAddressMode::Value addressModeR;
    GfxSamplerAddressMode::Value addressModeS;
    GfxSamplerAddressMode::Value addressModeT;

    GfxSamplerAddressMode::Value filterModeMin;
    GfxSamplerAddressMode::Value filterModeMag;
    GfxSamplerAddressMode::Value filterModeMip;

    float mipMinLevel;
    float mipMaxLevel;
    float maxAnisotropy;
};

m_decl_handle (HGfxBuffer, uint32_t)
m_decl_handle (HGfxColorTarget, uint32_t)
m_decl_handle (HGfxDepthTarget, uint32_t)
m_decl_handle (HGfxShaderSource, uint32_t)
m_decl_handle (HGfxRendShader, uint32_t)
m_decl_handle (HGfxDepthStencilState, uint32_t)
m_decl_handle (HGfxBlendState, uint32_t)
m_decl_handle (HGfxSampler, uint32_t)

class Frontend;

class GfxService
{
    m_noncopyable (GfxService)

public:
    GfxService() {}
    virtual ~GfxService() {}

    virtual bool startup (Frontend& frontEnd) = 0;
    virtual void shutdown() = 0;

// Frame Control
    virtual bool frameBegin() = 0;
    virtual void frameEnd() = 0;

// Render Targets
    //! Return the handle of the default color target
    virtual HGfxColorTarget colorTargetDefault() = 0;

    //! Clear a color target
    virtual void colorTargetClear (HGfxColorTarget target, float r, float g, float b, float a) = 0;

    //! Return the handle of the default depth target
    virtual HGfxDepthTarget depthTargetDefault() = 0;

    //! Clear a depth target
    virtual void depthTargetClear (HGfxDepthTarget target, float depth) = 0;

// Buffers
    virtual HGfxBuffer bufferCreateVertex (size_t sizeInBytes, uint32_t flags, const void* initialData = nullptr) = 0;
    virtual HGfxBuffer bufferCreateIndex (size_t sizeInBytes, uint32_t flags, const void* initialData = nullptr) = 0;
    virtual HGfxBuffer bufferCreateConstant (size_t sizeInBytes) = 0;
    virtual bool bufferDestroy (HGfxBuffer buffer) = 0;
    virtual bool bufferUpdate (HGfxBuffer buffer, const void* data, size_t dataSize, bool dynamic) = 0;

// Shader Source
    virtual HGfxShaderSource shaderSourceCreate (const void* dataPtr, size_t dataSize) = 0;
    virtual bool shaderSourceDestroy (HGfxShaderSource source) = 0;

// Rendering Shaders
    virtual HGfxRendShader rendShaderCreate (HGfxShaderSource vertexSrc, HGfxShaderSource fragmentSrc) = 0;
    virtual bool rendShaderApply (HGfxRendShader shader) = 0;
    virtual bool rendShaderDestroy (HGfxRendShader shader) = 0;

// Render States
    virtual HGfxDepthStencilState depthStencilStateCreate (GfxStencilDesc desc) = 0;
    virtual void depthStencilStateApply (HGfxDepthStencilState state) = 0;
    virtual void depthStencilStateDestroy (HGfxDepthStencilState state) = 0;

    virtual HGfxBlendState blendStateCreate (GfxBlendDesc desc) = 0;
    virtual void blendStateApply (HGfxBlendState state) = 0;
    virtual void blendStateDestroy (HGfxBlendState state) = 0;
    virtual void blendStateSetFactor (float r, float g, float b, float a) = 0;

    virtual HGfxSampler samplerCreate (GfxSamplerDesc desc) = 0;
    virtual void samplerDestroy (GfxSamplerDesc state) = 0;

};

}

#endif // MDK_GRAPHICS_H_INCLUDED
