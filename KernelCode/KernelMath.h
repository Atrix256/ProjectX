/*==================================================================================================

SharedMath.h

Math routines shared by both kernel and host code (actually host code doesnt like it. MatrixMath.h)

==================================================================================================*/

#pragma once

#include "Shared/SharedTypes.h"

inline void TransformPointByMatrix (
	float3 *outPoint,
	const float3 *inPoint,
	const __global cl_float4 *xAxis,
	const __global cl_float4 *yAxis,
	const __global cl_float4 *zAxis,
	const __global cl_float4 *wAxis)
{
	outPoint->x = inPoint->x * xAxis->x
				+ inPoint->y * yAxis->x
				+ inPoint->z * zAxis->x
				+		1.0f * wAxis->x;

	outPoint->y = inPoint->x * xAxis->y
				+ inPoint->y * yAxis->y
				+ inPoint->z * zAxis->y
				+		1.0f * wAxis->y;

	outPoint->z = inPoint->x * xAxis->z
				+ inPoint->y * yAxis->z
				+ inPoint->z * zAxis->z
				+		1.0f * wAxis->z;
}

inline void TransformPointByMatrixNoTemporary (
	float3 *point,
	const __global cl_float4 *xAxis,
	const __global cl_float4 *yAxis,
	const __global cl_float4 *zAxis,
	const __global cl_float4 *wAxis)
{
	float3 transformedPoint;
	TransformPointByMatrix(&transformedPoint, point, xAxis, yAxis, zAxis, wAxis);
	*point = transformedPoint;
}

inline void TransformVectorByMatrix (
	float3 *outPoint,
	const float3 *inPoint,
	const __global cl_float4 *xAxis,
	const __global cl_float4 *yAxis,
	const __global cl_float4 *zAxis)
{
	outPoint->x = inPoint->x * xAxis->x
				+ inPoint->y * yAxis->x
				+ inPoint->z * zAxis->x;
				//+		0.0f * wAxis->x;

	outPoint->y = inPoint->x * xAxis->y
				+ inPoint->y * yAxis->y
				+ inPoint->z * zAxis->y;
				//+		0.0f * wAxis->y;

	outPoint->z = inPoint->x * xAxis->z
				+ inPoint->y * yAxis->z
				+ inPoint->z * zAxis->z;
				//+		0.0f * wAxis->z;
}

inline void TransformVectorByMatrixNoTemporary (
	float3 *point,
	const __global cl_float4 *xAxis,
	const __global cl_float4 *yAxis,
	const __global cl_float4 *zAxis)
{
	float3 transformedPoint;
	TransformVectorByMatrix(&transformedPoint, point, xAxis, yAxis, zAxis);
	*point = transformedPoint;
}

inline float3 reflect(float3 V, float3 N)
{
	return V - 2.0f * dot( V, N ) * N;
}

inline float3 refract(float3 V, float3 N, float refrIndex)
{
	float cosI = -dot( N, V );
	float cosT2 = 1.0f - refrIndex * refrIndex * (1.0f - cosI * cosI);
	return (refrIndex * V) + (refrIndex * cosI - sqrt( cosT2 )) * N;
}

inline float3 GetSectorPlaneNormal(unsigned int planeIndex)
{
	switch (planeIndex)
	{
		// positive x
		case 0: return (float3)(-1.0f, 0.0f, 0.0f);
		//  negative x
		case 1: return (float3)(1.0f, 0.0f, 0.0f);
		// positive y
		case 2: return (float3)(0.0f,-1.0f, 0.0f);
		// negative y
		case 3: return (float3)(0.0f,1.0f, 0.0f);
		// positive z
		case 4: return (float3)(0.0f,0.0f,-1.0f);
		// negative z
		case 5: return (float3)(0.0f,0.0f, 1.0f);
	}
	return (float3)(0.0f, 0.0f, 0.0f);
}

inline float3 GetSectorPlaneU(unsigned int planeIndex)
{
	switch (planeIndex)
	{
		// positive x
		case 0: return (float3)( 0.0f, 0.0f,-1.0f);
		//  negative x
		case 1: return (float3)( 0.0f, 0.0f, 1.0f);
		// positive y
		case 2: return (float3)( 1.0f, 0.0f, 0.0f);
		// negative y
		case 3: return (float3)(-1.0f, 0.0f, 0.0f);
		// positive z
		case 4: return (float3)( 1.0f, 0.0f, 0.0f);
		// negative z
		case 5: return (float3)(-1.0f, 0.0f, 0.0f);
	}
	return (float3)(0.0f, 0.0f, 0.0f);
}

inline float3 GetSectorPlaneV(unsigned int planeIndex)
{
	switch (planeIndex)
	{
		// positive x
		case 0: return (float3)( 0.0f, 1.0f, 0.0f);
		//  negative x
		case 1: return (float3)( 0.0f, 1.0f, 0.0f);
		// positive y
		case 2: return (float3)( 0.0f, 0.0f,-1.0f);
		// negative y
		case 3: return (float3)( 0.0f, 0.0f, 1.0f);
		// positive z
		case 4: return (float3)( 0.0f, 1.0f, 0.0f);
		// negative z
		case 5: return (float3)( 0.0f, 1.0f, 0.0f);
	}
	return (float3)(0.0f, 0.0f, 0.0f);
}

inline float ColorToGray (float3 *color)
{
	return color->x * 0.3f + color->y * 0.59f + color->z * 0.11f;
}

inline float Saturate (float x)
{
	return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

inline float __OVERLOADABLE__ abs (float x)
{
	return x < 0.0f ? x * -1.0f : x;
}

inline float dotVectorPlane (const float3 *vector, __global const float4 *plane)
{
	// for vectors, the w component is zero
	return (vector->x * plane->x) +
		(vector->y * plane->y) +
		(vector->z * plane->z);
}

inline float dotPointPlane (const float3 *point, __global const float4 *plane)
{
	// for points, the w component is one
	return (point->x * plane->x) +
		(point->y * plane->y) +
		(point->z * plane->z) +
		(1.0f * plane->w);
}