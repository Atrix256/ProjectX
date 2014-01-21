/*==================================================================================================

SharedMath.h

Math routines shared by both kernel and host code (actually host code doesnt like it. MatrixMath.h)

==================================================================================================*/

#pragma once

#include "SharedTypes.h"

inline void TransformPointByMatrix (
	float3 *outPoint,
	const float3 *inPoint,
	const __constant cl_float4 *xAxis,
	const __constant cl_float4 *yAxis,
	const __constant cl_float4 *zAxis,
	const __constant cl_float4 *wAxis)
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

inline void TransformVectorByMatrix (
	float3 *outPoint,
	const float3 *inPoint,
	const __constant cl_float4 *xAxis,
	const __constant cl_float4 *yAxis,
	const __constant cl_float4 *zAxis)
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