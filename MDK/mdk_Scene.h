#ifndef MDK_SCENE_H_INCLUDED
#define MDK_SCENE_H_INCLUDED

#include "mdk_Config.h"
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
    uint32 frameCount;
    uint32 frameDataSize;
    float* frameTime;
    float* frameData;
    
    AnimationTrack (Allocator& allocator);
    ~AnimationTrack();

    Allocator& getAllocator()
    {
        return _allocator;
    }

    void alloc (uint32 numOfFrames, uint32 numOfElemsPerFrame);
    void dealloc();

    void setFrameTime (uint32 offset, uint32 numOfFrames, float* ptr);
    void setFrameData (uint32 offset, uint32 numOfFrames, float* ptr);

    void fetch2Frames (float* retFrameData, float* retFrameTime, float frameNo);

    static void _swap (AnimationTrack& a, AnimationTrack& b);

private:
    Allocator& _allocator;
};

//! specialize swap<>() for Manager<>
template<>
void swap<AnimationTrack> (AnimationTrack& a, AnimationTrack& b)
{
    return AnimationTrack::_swap (a, b);
}

template<> struct UseAllocator<AnimationTrack>
{
    static const bool value = true;
};

class AnimationTrackManager : public Manager< ManagerTraitsDefault<AnimationTrack> >
{
    m_noncopyable (AnimationTrackManager)

    typedef Manager< ManagerTraitsDefault<AnimationTrack> > Super;

public:
    AnimationTrackManager (Allocator& allocator);

};

typedef AnimationTrackManager::Handle AnimationTrackHandle;

}

#endif // MDK_SCENE_H_INCLUDED
