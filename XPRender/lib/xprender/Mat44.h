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

XprMat44 XprMat44_(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33);

void XprMat44_mult(XprMat44* _out, const XprMat44* a, const XprMat44* b);

void XprMat44_transpose(XprMat44* _out, const XprMat44* m);

void XprMat44_transform(struct XprVec4* _out, const XprMat44* m);

void XprMat44_transformAffineDir(struct XprVec3* _out, const XprMat44* m);

void XprMat44_transformAffinePt(struct XprVec3* _out, const XprMat44* m);

void XprMat44_setIdentity(XprMat44* _out);

void XprMat44_setTranslation(XprMat44* _out, const struct XprVec3* v);

void XprMat44_getTranslation(struct XprVec3* v, const XprMat44* m);

void XprMat44_getBasis(struct XprVec3* xaxis, struct XprVec3* yaxis, struct XprVec3* zaxis, const XprMat44* m);

void XprMat44_cameraLookAt(XprMat44* _out, const struct XprVec3* eyeAt, const struct XprVec3* lookAt, const struct XprVec3* eyeUp);

void XprMat44_prespective(XprMat44* _out, float fovyDeg, float aspect, float znear, float zfar);

void XprMat44_planarReflect(XprMat44* _out, const struct XprVec3* normal, const struct XprVec3* point);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_MAT44_H__