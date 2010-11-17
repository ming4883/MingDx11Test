#ifndef __XPRENDER_MAT44_H__
#define __XPRENDER_MAT44_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xprVec3 xprVec3;
typedef struct xprVec4 xprVec4;

typedef struct xprMat44
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

} xprMat44;

xprMat44 xprMat44_(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33);

void xprMat44_mult(xprMat44* _out, const xprMat44* a, const xprMat44* b);

void xprMat44_transpose(xprMat44* _out, const xprMat44* m);

void xprMat44_transform(xprVec4* _out, const xprMat44* m);

void xprMat44_transformAffineDir(xprVec3* _out, const xprMat44* m);

void xprMat44_transformAffinePt(xprVec3* _out, const xprMat44* m);

void xprMat44_setIdentity(xprMat44* _out);

void xprMat44_setTranslation(xprMat44* _out, const float v[3]);

void xprMat44_cameraLookAt(xprMat44* _out, const xprVec3* eyeAt, const xprVec3* lookAt, const xprVec3* eyeUp);

void xprMat44_planarReflect(xprMat44* _out, const xprVec3* normal, const xprVec3* point);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_MAT44_H__