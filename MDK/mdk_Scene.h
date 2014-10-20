#ifndef MDK_SCENE_H_INCLUDED
#define MDK_SCENE_H_INCLUDED

#include "mdk_Allocator.h"
#include "mdk_Math.h"
#include "mdk_Threading.h"

using juce::uint32;

namespace mdk
{

struct CameraRenderData
{
    Mat44f viewMatrix;              //!< derived from transform
    Mat44f projectionMatrix;        //!< derived from projection
    Mat44f projectionviewMatrix;    //!< derived from viewMatrix and viewprojectionMatrix
};

class Camera
{
public:
    struct Projection
    {
        float fovY;
        float aspect;
        float zNear;
        float zFar;
    };

    Synced<Transform3f> transform;
    Synced<Projection> projection;

    Mat44f viewMatrix;              //!< derived from transform
    Mat44f projectionMatrix;        //!< derived from projection
    Mat44f projectionviewMatrix;    //!< derived from viewMatrix and viewprojectionMatrix

    Camera();

    void updateForD3D (float rtAspect = 1.0f);
};

class Animation
{
    m_noncopyable (Animation)

public:
    Animation (Allocator& allocator);
    ~Animation();

    Allocator& getAllocator() { return _allocator; }

    float* frameTime;
    float* frameData;
    uint32 frameCount;
    uint32 frameDataSize;

    void alloc (uint32 numOfFrames, uint32 numOfElemsPerFrame);
    void dealloc();

    void fetch2Frames (float* retFrameData, float* retFrameTime, float frameNo);

private:
    Allocator& _allocator;
};

template<> struct UseAllocator<Animation> { static const bool value = true; };

}

#endif // MDK_SCENE_H_INCLUDED
