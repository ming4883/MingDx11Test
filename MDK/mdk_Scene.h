#ifndef MDK_SCENE_H_INCLUDED
#define MDK_SCENE_H_INCLUDED

#include "mdk_Math.h"
#include "mdk_Threading.h"

namespace mdk
{

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

}

#endif // MDK_SCENE_H_INCLUDED
