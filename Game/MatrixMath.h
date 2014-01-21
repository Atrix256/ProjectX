/*==================================================================================================

MatrixMath.h

Matrix math routines used by host code

==================================================================================================*/

#pragma once

#include "Platform/float3.h"

inline void TransformPointByMatrix (
	float3 &outPoint,
	const float3 &inPoint,
	const cl_float4 &xAxis,
	const cl_float4 &yAxis,
	const cl_float4 &zAxis,
	const cl_float4 &wAxis)
{
	outPoint[0] = inPoint[0] * xAxis.s[0]
				+ inPoint[1] * yAxis.s[0]
				+ inPoint[2] * zAxis.s[0]
				+		1.0f * wAxis.s[0];

	outPoint[1] = inPoint[0] * xAxis.s[1]
				+ inPoint[1] * yAxis.s[1]
				+ inPoint[2] * zAxis.s[1]
				+		1.0f * wAxis.s[1];

	outPoint[2] = inPoint[0] * xAxis.s[2]
				+ inPoint[1] * yAxis.s[2]
				+ inPoint[2] * zAxis.s[2]
				+		1.0f * wAxis.s[2];
}

inline void TransformVectorByMatrix (
	float3 &outPoint,
	const float3 &inPoint,
	const cl_float4 &xAxis,
	const cl_float4 &yAxis,
	const cl_float4 &zAxis)
{
	outPoint[0] = inPoint[0] * xAxis.s[0]
				+ inPoint[1] * yAxis.s[0]
				+ inPoint[2] * zAxis.s[0];
				//+		0.0f * wAxis->x;

	outPoint[1] = inPoint[0] * xAxis.s[1]
				+ inPoint[1] * yAxis.s[1]
				+ inPoint[2] * zAxis.s[1];
				//+		0.0f * wAxis.s[1];

	outPoint[2] = inPoint[0] * xAxis.s[2]
				+ inPoint[1] * yAxis.s[2]
				+ inPoint[2] * zAxis.s[2];
				//+		0.0f * wAxis.s[2];
}