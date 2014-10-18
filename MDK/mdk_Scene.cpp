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

}   // namespace
