#include "Mat44.h"

void xprMat44AdjustToAPIDepthRange(XprMat44* _out)
{
	_out->m20 = (_out->m20 + _out->m30) * 0.5f;  
	_out->m21 = (_out->m21 + _out->m31) * 0.5f;
	_out->m22 = (_out->m22 + _out->m32) * 0.5f;
	_out->m23 = (_out->m23 + _out->m33) * 0.5f;
}