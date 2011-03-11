#ifndef __XPRENDER_MAT44_H__
#define __XPRENDER_MAT44_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprVec3;

struct XprVec4;

typedef struct XprMat44
{
	union
	{
		struct { float
			m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33;
		};

		float v[16];
	};

} XprMat44;

XprMat44 xprMat44(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33);

void xprMat44Mult(XprMat44* _out, const XprMat44* a, const XprMat44* b);

XprBool xprMat44Inverse(XprMat44* _out, const XprMat44* m);

void xprMat44Transpose(XprMat44* _out, const XprMat44* m);

void xprMat44Transform(struct XprVec4* _out, const XprMat44* m);

//! assume the plane equation is stored as A, B, C, D
void xprMat44TransformPlane(struct XprVec4* _out, const XprMat44* m);

void xprMat44TransformAffineDir(struct XprVec3* _out, const XprMat44* m);

void xprMat44TransformAffinePt(struct XprVec3* _out, const XprMat44* m);

void xprMat44SetIdentity(XprMat44* _out);

void xprMat44SetTranslation(XprMat44* _out, const struct XprVec3* v);

void xprMat44GetTranslation(struct XprVec3* v, const XprMat44* m);

void xprMat44MakeTranslation(XprMat44* _out, const struct XprVec3* v);

void xprMat44MakeScale(XprMat44* _out, const struct XprVec3* v);

void xprMat44MakeRotationX(XprMat44* _out, float angleInDeg);

void xprMat44MakeRotationY(XprMat44* _out, float angleInDeg);

void xprMat44MakeRotationZ(XprMat44* _out, float angleInDeg);

void xprMat44MakeRotation(XprMat44* _out, const struct XprVec3* axis, float angleInDeg);

void xprMat44GetBasis(struct XprVec3* xaxis, struct XprVec3* yaxis, struct XprVec3* zaxis, const XprMat44* m);

void xprMat44CameraLookAt(XprMat44* _out, const struct XprVec3* eyeAt, const struct XprVec3* lookAt, const struct XprVec3* eyeUp);

void xprMat44Prespective(XprMat44* _out, float fovyDeg, float aspect, float znear, float zfar);

void xprMat44PlanarReflect(XprMat44* _out, const struct XprVec3* normal, const struct XprVec3* point);

//! adjust a projection matrix to match the API's depth range
void xprMat44AdjustToAPIDepthRange(XprMat44* _out);

//! adjust a projection matrix to match the API's projective texture lookup
void xprMat44AdjustToAPIProjectiveTexture(XprMat44* _out);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_MAT44_H__