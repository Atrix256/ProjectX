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

	inline float3 operator+= (const float3& other)
	{
		(*this)[0] += other[0];
		(*this)[1] += other[1];
		(*this)[2] += other[2];
		return *this;
	}

	inline float3 operator- (const float3& other) const
	{
		float3 ret;
		ret[0] = m_data.s[0] - other[0];
		ret[1] = m_data.s[1] - other[1];
		ret[2] = m_data.s[2] - other[2];
		return ret;
	}

	inline float3 operator-= (const float3& other)
	{
		(*this)[0] -= other[0];
		(*this)[1] -= other[1];
		(*this)[2] -= other[2];
		return *this;
	}

	inline float3 operator/ (const float v) const
	{
		float3 ret;
		ret[0] = m_data.s[0] / v;
		ret[1] = m_data.s[1] / v;
		ret[2] = m_data.s[2] / v;
		return ret;
	}

	inline float3 operator/= (const float v)
	{
		(*this)[0] /= v;
		(*this)[1] /= v;
		(*this)[2] /= v;
		return *this;
	}

	inline float3 operator* (const float v) const
	{
		float3 ret;
		ret[0] = m_data.s[0] * v;
		ret[1] = m_data.s[1] * v;
		ret[2] = m_data.s[2] * v;
		return ret;
	}

	inline float3 operator*= (const float v)
	{
		(*this)[0] *= v;
		(*this)[1] *= v;
		(*this)[2] *= v;
		return *this;
	}

	cl_float3 m_data;
};

#ifndef OPENCL
inline float dot(const float3& A, const float3& B)
{
	return A[0]*B[0] + A[1]*B[1] + A[2]*B[2];
}
#endif

inline float lengthsq (const float3& v)
{
	return dot(v, v);
}

inline float length (const float3& v)
{
	return sqrt(lengthsq(v));
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

inline cl_float4 plane (const float3& normal, const float3& point)
{
	cl_float4 ret;
	ret.s[0] = normal[0];
	ret.s[1] = normal[1];
	ret.s[2] = normal[2];
	ret.s[3] = dot(normal, point);
	return ret;
}