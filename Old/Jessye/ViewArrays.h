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
	size_t m_Count;

	ViewArray_t() : m_Current(&m_Array[0]), m_Count(0) {}
	
	ViewArray_t& operator << (T srv)
	{
		if(m_Count >= N)
			return *this;

		*m_Current++ = srv;
		m_Count++;
		return *this;
	}

	operator T* () { return m_Array;}

};	// ViewArray_t

typedef ViewArray_t<ID3D11ShaderResourceView*, 16> SrvVA;
typedef ViewArray_t<ID3D11RenderTargetView*, 8> RtvVA;
typedef ViewArray_t<D3D11_VIEWPORT, 8> VpVA;
typedef ViewArray_t<unsigned int, 16> UintVA;
typedef ViewArray_t<ID3D11Buffer*, 16> BufVA;
typedef ViewArray_t<ID3D11SamplerState*, 16> SampVA;
}	// namespace js

#endif	// VIEWARRAYS_H