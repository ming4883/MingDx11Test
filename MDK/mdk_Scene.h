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
    enum { cNumOfFrames = 2 };
    float time[cNumOfFrames];
    Vec4f data[cNumOfFrames];

    AnimationCache()
    {
        time[0] = time[1] = -1;
    }

    m_inline bool isValid (float t) const
    {
        return !((time[0] == -1) || (time[0] < t) || (time[1] > t));
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

    void fetch2Frames (AnimationCache& ret, float frameNo);

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

class AnimationTrackManager : protected SOAManager< SOAManagerTraitsDefault<AnimationTrack> >
{
    m_noncopyable (AnimationTrackManager)

public:
    typedef SOAManager< SOAManagerTraitsDefault<AnimationTrack> > Super;
    typedef SOAColumn<Super, 0> ColTrack;

    using Super::Handle;
    using Super::disable;
    using Super::enable;
    using Super::isValid;

    AnimationTrackManager (Allocator& allocator);

    m_inline AnimationTrack* get (Handle handle)
    {
        return Super::get<ColTrack> (handle);
    }

    Handle create (uint32 numOfFrames, uint32 numOfElemsPerFrame);

    bool destroy (Handle handle);
};

typedef AnimationTrackManager::Handle AnimationTrackHandle;

class AnimationState
{
public:
    float time;
    AnimationCache cache;
    AnimationTrackHandle track;
};

class AnimationResult
{
public:
    Vec4f data;
};

class AnimationStateManager : protected SOAManager< SOAManagerTraitsDefault<AnimationState, AnimationResult> >
{
    m_noncopyable (AnimationStateManager)

public:
    typedef SOAManager< SOAManagerTraitsDefault<AnimationState, AnimationResult> > Super;
    typedef SOAColumn<Super, 0> ColState;
    typedef SOAColumn<Super, 1> ColResult;

    using Super::Handle;
    
    AnimationStateManager (Allocator& allocator);

    void update (AnimationTrackManager& trackManager);

private:
    
};

typedef AnimationStateManager::Handle AnimationStateHandle;

}

#endif // MDK_SCENE_H_INCLUDED
