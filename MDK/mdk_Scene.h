#ifndef MDK_SCENE_H_INCLUDED
#define MDK_SCENE_H_INCLUDED

#include "mdk_Config.h"
#include "mdk_Allocator.h"
#include "mdk_Math.h"
#include "mdk_SOAManager.h"
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

class AnimationCache
{
public:
    enum { cNumOfFrames = 4 };
    float time[cNumOfFrames];
    float data[cNumOfFrames * 4];

    AnimationCache()
    {
        time[0] = time[1] = time[2] = time[3] = -1;
    }

    m_inline bool isValid() const
    {
        return false;
    }
};

class AnimationTrack
{
    m_noncopyable (AnimationTrack)

public:
    uint32 frameCount;
    uint32 frameDataSize;
    float* frameTimePtr;
    float* frameDataPtr;
    
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

//! specialize swap<>() for SOAManager<>
template<>
void swap<AnimationTrack> (AnimationTrack& a, AnimationTrack& b)
{
    return AnimationTrack::_swap (a, b);
}

template<> struct UseAllocator<AnimationTrack>
{
    static const bool Value = true;
};

class AnimationTrackManager : public SOAManager< SOAManagerTraitsDefault<AnimationTrack> >
{
    m_noncopyable (AnimationTrackManager)

    typedef SOAManager< SOAManagerTraitsDefault<AnimationTrack> > Super;

public:
    AnimationTrackManager (Allocator& allocator);
};

typedef AnimationTrackManager::Handle AnimationTrackHandle;

class AnimationState
{
public:
    float time;
    AnimationCache cache;
    AnimationTrackHandle track;
    float result[4];
};

class AnimationStateManager : public SOAManager< SOAManagerTraitsDefault<AnimationState> >
{
    m_noncopyable (AnimationStateManager)

    typedef SOAManager< SOAManagerTraitsDefault<AnimationState> > Super;

public:
    AnimationStateManager (Allocator& allocator);

    void update (AnimationTrackManager& trackManager);

};

typedef AnimationStateManager::Handle AnimationStateHandle;

}

#endif // MDK_SCENE_H_INCLUDED
