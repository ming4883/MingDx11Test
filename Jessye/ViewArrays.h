#ifndef VIEWARRAYS_H
#define VIEWARRAYS_H

#include "Platform.h"

namespace js
{
	
template<typename T, int N = 16>
struct ViewArray_t
{
	T m_Array[N];
	T* m_Current;

	ViewArray_t() : m_Current(&m_Array[0]) {}
	
	ViewArray_t& operator << (T srv)
	{
		*m_Current++ = srv;
		return *this;
	}

	operator T* () { return m_Array;}

};	// ViewArray_t

typedef ViewArray_t<ID3D11ShaderResourceView*, 16> SrvVA;
typedef ViewArray_t<ID3D11RenderTargetView*, 16> RtvVA;
typedef ViewArray_t<unsigned int, 16> UintVA;
typedef ViewArray_t<ID3D11Buffer*, 16> BufVA;

}	// namespace js

#endif	// VIEWARRAYS_H