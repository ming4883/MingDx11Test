#ifndef MDK_SCENE_H_INCLUDED
#define MDK_SCENE_H_INCLUDED

#include "mdk_Allocator.h"
#include "mdk_Manager.h"
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

class AnimationTrack
{
    m_noncopyable (AnimationTrack)

public:
    float* frameTime;
    float* frameData;
    uint32 frameCount;
    uint32 frameDataSize;

    AnimationTrack (Allocator& allocator);
    ~AnimationTrack();

    Allocator& getAllocator() { return _allocator; }

    void alloc (uint32 numOfFrames, uint32 numOfElemsPerFrame);
    void dealloc();

    void setFrameTime (uint32 offset, uint32 numOfFrames, float* ptr);
    void setFrameData (uint32 offset, uint32 numOfFrames, float* ptr);

    void fetch2Frames (float* retFrameData, float* retFrameTime, float frameNo);

private:
    Allocator& _allocator;
};

template<> struct UseAllocator<AnimationTrack> { static const bool value = true; };

class SceneAnimation
{
public:

};

class SceneAnimationManager : public Manager< ManagerTraitsDefault<SceneAnimation> >
{
    m_noncopyable (SceneAnimationManager)

    typedef Manager< ManagerTraitsDefault<SceneAnimation> > Super;

public:
    SceneAnimationManager (Allocator& allocator);

};

}

#endif // MDK_SCENE_H_INCLUDED
