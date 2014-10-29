#include "mdk_Scene.h"

namespace mdk
{


//==============================================================================
Camera::Camera()
{
    Transform3f tran;
    Transform::setIdentity (tran);
    transform.commit (tran);

    Projection proj;
    proj.aspect = 1.0f;
    proj.fovY = Scalar::rad<float> (45.0f);
    proj.zFar = 1025.0f;
    proj.zNear = 1.0f;
    projection.commit (proj);
}

void Camera::updateForD3D (float rtAspect)
{
    Transform3f tran = transform.fetch();
    Projection proj = projection.fetch();

    Transform3f inv;
    Transform::inverse (inv, tran);
    Mat::fromTransform3 (viewMatrix, inv);

    float aspect = rtAspect * proj.aspect;
    float zn = proj.zNear;
    float zf = proj.zFar;
    float sinFov, cosFov;
    Scalar::calcSinCos<float> (sinFov, cosFov, proj.fovY * 0.5f);

    float yscale = cosFov / sinFov;
    float xscale = yscale / aspect;
    float zscale = 1.0f / (zn - zf);

    projectionMatrix[0][0] = -xscale;
    projectionMatrix[0][1] = 0;
    projectionMatrix[0][2] = 0;
    projectionMatrix[0][3] = 0;

    projectionMatrix[1][0] = 0;
    projectionMatrix[1][1] = yscale;
    projectionMatrix[1][2] = 0;
    projectionMatrix[1][3] = 0;

    projectionMatrix[2][0] = 0;
    projectionMatrix[2][1] = 0;
    projectionMatrix[2][2] = zf * zscale;
    projectionMatrix[2][3] = zn * zf * zscale;

    projectionMatrix[3][0] = 0;
    projectionMatrix[3][1] = 0;
    projectionMatrix[3][2] =-1;
    projectionMatrix[3][3] = 0;

    Mat::mul (projectionviewMatrix, projectionMatrix, viewMatrix);
}

//==============================================================================
AnimationTrack::AnimationTrack (Allocator& allocator)
    : _allocator (allocator)
    , frameCount (0)
    , frameDataSize (0)
    , frameTimePtr (nullptr)
    , frameDataPtr (nullptr)
{

}

AnimationTrack::~AnimationTrack()
{
    dealloc();
}

void AnimationTrack::alloc (uint32 numOfFrames, uint32 numOfElemsPerFrame)
{
    m_assert (numOfFrames > 1);
    m_assert (numOfElemsPerFrame > 0);

    frameCount = numOfFrames;
    frameDataSize = numOfElemsPerFrame;

    frameTimePtr = static_cast<float*> (_allocator.malloc (sizeof (float) * numOfFrames));
    frameDataPtr = static_cast<float*> (_allocator.malloc (sizeof (float) * numOfFrames * numOfElemsPerFrame));
}

void AnimationTrack::dealloc()
{
    _allocator.free (frameTimePtr);
    _allocator.free (frameDataPtr);
}

void AnimationTrack::setFrameTime (uint32 offset, uint32 numOfFrames, float* ptr)
{
    for (uint32 i = 0; i < numOfFrames; ++i)
    {
        frameTimePtr[offset + i] = ptr[i];
    }
}

void AnimationTrack::setFrameData (uint32 offset, uint32 numOfFrames, float* ptr)
{
    float* dstPtr = frameDataPtr;
    float* srcPtr = ptr;

    uint32 srcOffset = 0;
    uint32 dstOffset = offset * frameDataSize;

    if (1 == frameDataSize)
    {
        for (uint32 i = 0; i < numOfFrames; ++i)
        {
            dstPtr[dstOffset++] = srcPtr[srcOffset++];
        }
    }
    else if (2 == frameDataSize)
    {
        for (uint32 i = 0; i < numOfFrames; ++i)
        {
            dstPtr[dstOffset++] = srcPtr[srcOffset++];
            dstPtr[dstOffset++] = srcPtr[srcOffset++];
        }
    }
    else if (3 == frameDataSize)
    {
        for (uint32 i = 0; i < numOfFrames; ++i)
        {
            dstPtr[dstOffset++] = srcPtr[srcOffset++];
            dstPtr[dstOffset++] = srcPtr[srcOffset++];
            dstPtr[dstOffset++] = srcPtr[srcOffset++];
        }
    }
    else if (4 == frameDataSize)
    {
        for (uint32 i = 0; i < numOfFrames; ++i)
        {
            dstPtr[dstOffset++] = srcPtr[srcOffset++];
            dstPtr[dstOffset++] = srcPtr[srcOffset++];
            dstPtr[dstOffset++] = srcPtr[srcOffset++];
            dstPtr[dstOffset++] = srcPtr[srcOffset++];
        }
    }
    else
    {
        for (uint32 i = 0; i < numOfFrames; ++i)
        {
            for (uint32 j = 0; j < frameDataSize; ++j)
                dstPtr[dstOffset++] = srcPtr[srcOffset++];
        }
    }
}

void AnimationTrack::fetch2Frames (float* retFrameData, float* retFrameTime, float frameNo)
{
    uint32 thisFrame = 0;

    while (frameTimePtr[thisFrame] < frameNo && thisFrame < frameCount)
    {
        thisFrame++;
    }

    if (thisFrame >= frameCount)
        thisFrame = frameCount - 1;

    uint32 lastFrame;
    if (thisFrame == 0)
        lastFrame = 0;
    else
        lastFrame = thisFrame - 1;

    uint32 srcOffset;
    uint32 dstOffset;

    // first frame
    srcOffset = lastFrame * frameDataSize;
    dstOffset = 0;

    for (uint32 i = 0; i < frameDataSize; ++i)
        retFrameData[dstOffset + i] = frameDataPtr[srcOffset + i];

    // second frame
    srcOffset = thisFrame * frameDataSize;
    dstOffset = frameDataSize;

    for (uint32 i = 0; i < frameDataSize; ++i)
        retFrameData[dstOffset + i] = frameDataPtr[srcOffset + i];

    retFrameTime[0] = frameTimePtr[lastFrame];
    retFrameTime[1] = frameTimePtr[thisFrame];
}

void AnimationTrack::_swap (AnimationTrack& a, AnimationTrack& b)
{
    swap (a.frameCount, b.frameCount);
    swap (a.frameDataSize, b.frameDataSize);
    swap (a.frameTimePtr, b.frameTimePtr);
    swap (a.frameDataPtr, b.frameDataPtr);
}

//==============================================================================
AnimationTrackManager::AnimationTrackManager (Allocator& allocator)
    : Super (16u, allocator)
{

}

//==============================================================================
AnimationStateManager::AnimationStateManager (Allocator& allocator)
    : Super (16u, allocator)
{

}

void AnimationStateManager::update (AnimationTrackManager& trackManager)
{
#if 0
    AnimationState* const itBeg = beginOfEnabled();
    AnimationState* const itEnd = endOfEnabled();

    for (AnimationState* itCur = itBeg; itCur != itEnd; ++itCur)
    {
        if (!itCur->cache.isValid())
        {
            AnimationTrack* track = trackManager.get (itCur->track);
            m_assert (track != nullptr);
            track->fetch2Frames (itCur->cache.data, itCur->cache.time, itCur->time);
        }
    }
#endif
}

}   // namespace
