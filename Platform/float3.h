/*==================================================================================================

float3.h

This defines a float3 that can be used on the C++ side and is compatible with float3 OpenCL side

==================================================================================================*/

#pragma once

//#ifndef OPENCL <--- this is here in case we need to share code with the .cl file

// not using cl_float3 because cl_float3 is really a typedef for cl_float4 which makes it
// impossible to make functions like normalize() that rely on overloading to know how many
// elements are in the float

#include "oclUtils.h"
#include <math.h>

struct float3 {
public:
	inline float& operator[] (int i)
	{
		return m_data.s[i];
	}

	inline float operator[] (int i) const
	{
		return m_data.s[i];
	}

	inline float3& operator= (const float v[3])
	{
		m_data.s[0] = v[0];
		m_data.s[0] = v[0];
		m_data.s[0] = v[0];
	}

	inline float3 operator+ (const float3& other) const
	{
		float3 ret;
		ret[0] = m_data.s[0] + other[0];
		ret[1] = m_data.s[1] + other[1];
		ret[2] = m_data.s[2] + other[2];
		return ret;
	}

	inline float3 operator- (const float3& other) const
	{
		float3 ret;
		ret[0] = m_data.s[0] - other[0];
		ret[1] = m_data.s[1] - other[1];
		ret[2] = m_data.s[2] - other[2];
		return ret;
	}

	inline float3 operator/ (const float v) const
	{
		float3 ret;
		ret[0] = m_data.s[0] / v;
		ret[1] = m_data.s[1] / v;
		ret[2] = m_data.s[2] / v;
		return ret;
	}

	inline float3 operator* (const float v) const
	{
		float3 ret;
		ret[0] = m_data.s[0] * v;
		ret[1] = m_data.s[1] * v;
		ret[2] = m_data.s[2] * v;
		return ret;
	}

	cl_float3 m_data;
};

inline float length (const float3& v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

inline float3 normalize (const float3& v)
{
	return v / length(v);
}

inline float3 cross (const float3& A, const float3& B)
{
	const float x1 = A[0];
	const float y1 = A[1];
	const float z1 = A[2];

	const float x2 = B[0];
	const float y2 = B[1];
	const float z2 = B[2];

	float3 ret;
	ret[0] = (y1 * z2) - (y2 * z1);
	ret[1] = (z1 * x2) - (z2 * x1);
	ret[2] = (x1 * y2) - (x2 * y1);
	return ret;
}