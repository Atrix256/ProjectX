/*==================================================================================================

MatrixMath.h

Matrix math routines used by host code

==================================================================================================*/

#pragma once

#include "Platform/float3.h"

//-----------------------------------------------------------------------------
inline void MatrixIdentity (
	cl_float4 &xAxis,
	cl_float4 &yAxis,
	cl_float4 &zAxis,
	cl_float4 &wAxis)
{
	xAxis.s[0] = 1.0f;
	xAxis.s[1] = 0.0f;
	xAxis.s[2] = 0.0f;
	xAxis.s[3] = 0.0f;

	yAxis.s[0] = 0.0f;
	yAxis.s[1] = 1.0f;
	yAxis.s[2] = 0.0f;
	yAxis.s[3] = 0.0f;

	zAxis.s[0] = 0.0f;
	zAxis.s[1] = 0.0f;
	zAxis.s[2] = 1.0f;
	zAxis.s[3] = 0.0f;

	wAxis.s[0] = 0.0f;
	wAxis.s[1] = 0.0f;
	wAxis.s[2] = 0.0f;
	wAxis.s[3] = 1.0f;
}

//-----------------------------------------------------------------------------
inline void MatrixTranslation (
	cl_float4 &xAxis,
	cl_float4 &yAxis,
	cl_float4 &zAxis,
	cl_float4 &wAxis,
	const float3 &translation)
{
	xAxis.s[0] = 1.0f;
	xAxis.s[1] = 0.0f;
	xAxis.s[2] = 0.0f;
	xAxis.s[3] = 0.0f;

	yAxis.s[0] = 0.0f;
	yAxis.s[1] = 1.0f;
	yAxis.s[2] = 0.0f;
	yAxis.s[3] = 0.0f;

	zAxis.s[0] = 0.0f;
	zAxis.s[1] = 0.0f;
	zAxis.s[2] = 1.0f;
	zAxis.s[3] = 0.0f;

	wAxis.s[0] = translation[0];
	wAxis.s[1] = translation[1];
	wAxis.s[2] = translation[2];
	wAxis.s[3] = 1.0f;
}

//-----------------------------------------------------------------------------
inline void MatrixScale (
	cl_float4 &xAxis,
	cl_float4 &yAxis,
	cl_float4 &zAxis,
	cl_float4 &wAxis,
	const float scale)
{
	xAxis.s[0] = scale;
	xAxis.s[1] = 0.0f;
	xAxis.s[2] = 0.0f;
	xAxis.s[3] = 0.0f;

	yAxis.s[0] = 0.0f;
	yAxis.s[1] = scale;
	yAxis.s[2] = 0.0f;
	yAxis.s[3] = 0.0f;

	zAxis.s[0] = 0.0f;
	zAxis.s[1] = 0.0f;
	zAxis.s[2] = scale;
	zAxis.s[3] = 0.0f;

	wAxis.s[0] = 0.0f;
	wAxis.s[1] = 0.0f;
	wAxis.s[2] = 0.0f;
	wAxis.s[3] = 1.0f;
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
inline void TransformPointByMatrix (
	float3 &point,
	const cl_float4 &xAxis,
	const cl_float4 &yAxis,
	const cl_float4 &zAxis,
	const cl_float4 &wAxis)
{
	float3 transformedPoint;
	TransformPointByMatrix(transformedPoint, point, xAxis, yAxis, zAxis, wAxis);
	point = transformedPoint;
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
inline void TransformVectorByMatrix (
	float3 &point,
	const cl_float4 &xAxis,
	const cl_float4 &yAxis,
	const cl_float4 &zAxis)
{
	float3 transformedPoint;
	TransformVectorByMatrix(transformedPoint, point, xAxis, yAxis, zAxis);
	point = transformedPoint;
}

//-----------------------------------------------------------------------------
inline void TransformPointOrVectorByMatrix (
	float3 &outPoint,
	const float3 &inPoint,
	const float3 &xAxis,
	const float3 &yAxis,
	const float3 &zAxis)
{
	outPoint[0] = inPoint[0] * xAxis[0]
				+ inPoint[1] * yAxis[0]
				+ inPoint[2] * zAxis[0];

	outPoint[1] = inPoint[0] * xAxis[1]
				+ inPoint[1] * yAxis[1]
				+ inPoint[2] * zAxis[1];

	outPoint[2] = inPoint[0] * xAxis[2]
				+ inPoint[1] * yAxis[2]
				+ inPoint[2] * zAxis[2];
}

//-----------------------------------------------------------------------------
inline void TransformPointOrVectorByMatrix (
	float3 &point,
	const float3 &xAxis,
	const float3 &yAxis,
	const float3 &zAxis)
{
	float3 transformedPoint;
	TransformPointOrVectorByMatrix(transformedPoint, point, xAxis, yAxis, zAxis);
	point = transformedPoint;
}

//-----------------------------------------------------------------------------
inline void TransformMatrixByMatrix (
	cl_float4 &outXAxis,
	cl_float4 &outYAxis,
	cl_float4 &outZAxis,
	cl_float4 &outWAxis,
	const cl_float4 &inXAxisA,
	const cl_float4 &inYAxisA,
	const cl_float4 &inZAxisA,
	const cl_float4 &inWAxisA,
	const cl_float4 &inXAxisB,
	const cl_float4 &inYAxisB,
	const cl_float4 &inZAxisB,
	const cl_float4 &inWAxisB)
{
	// X Axis
	outXAxis.s[0] = inXAxisA.s[0] * inXAxisB.s[0]
	              + inXAxisA.s[1] * inYAxisB.s[0]
	              + inXAxisA.s[2] * inZAxisB.s[0]
	              + inXAxisA.s[3] * inWAxisB.s[0];

	outXAxis.s[1] = inXAxisA.s[0] * inXAxisB.s[1]
	              + inXAxisA.s[1] * inYAxisB.s[1]
	              + inXAxisA.s[2] * inZAxisB.s[1]
	              + inXAxisA.s[3] * inWAxisB.s[1];

	outXAxis.s[2] = inXAxisA.s[0] * inXAxisB.s[2]
	              + inXAxisA.s[1] * inYAxisB.s[2]
	              + inXAxisA.s[2] * inZAxisB.s[2]
	              + inXAxisA.s[3] * inWAxisB.s[2];

	outXAxis.s[3] = inXAxisA.s[0] * inXAxisB.s[3]
	              + inXAxisA.s[1] * inYAxisB.s[3]
	              + inXAxisA.s[2] * inZAxisB.s[3]
	              + inXAxisA.s[3] * inWAxisB.s[3];

	// Y Axis
	outYAxis.s[0] = inYAxisA.s[0] * inXAxisB.s[0]
	              + inYAxisA.s[1] * inYAxisB.s[0]
	              + inYAxisA.s[2] * inZAxisB.s[0]
	              + inYAxisA.s[3] * inWAxisB.s[0];

	outYAxis.s[1] = inYAxisA.s[0] * inXAxisB.s[1]
	              + inYAxisA.s[1] * inYAxisB.s[1]
	              + inYAxisA.s[2] * inZAxisB.s[1]
	              + inYAxisA.s[3] * inWAxisB.s[1];

	outYAxis.s[2] = inYAxisA.s[0] * inXAxisB.s[2]
	              + inYAxisA.s[1] * inYAxisB.s[2]
	              + inYAxisA.s[2] * inZAxisB.s[2]
	              + inYAxisA.s[3] * inWAxisB.s[2];

	outYAxis.s[3] = inYAxisA.s[0] * inXAxisB.s[3]
	              + inYAxisA.s[1] * inYAxisB.s[3]
	              + inYAxisA.s[2] * inZAxisB.s[3]
	              + inYAxisA.s[3] * inWAxisB.s[3];

	// Z Axis
	outZAxis.s[0] = inZAxisA.s[0] * inXAxisB.s[0]
	              + inZAxisA.s[1] * inYAxisB.s[0]
	              + inZAxisA.s[2] * inZAxisB.s[0]
	              + inZAxisA.s[3] * inWAxisB.s[0];

	outZAxis.s[1] = inZAxisA.s[0] * inXAxisB.s[1]
	              + inZAxisA.s[1] * inYAxisB.s[1]
	              + inZAxisA.s[2] * inZAxisB.s[1]
	              + inZAxisA.s[3] * inWAxisB.s[1];

	outZAxis.s[2] = inZAxisA.s[0] * inXAxisB.s[2]
	              + inZAxisA.s[1] * inYAxisB.s[2]
	              + inZAxisA.s[2] * inZAxisB.s[2]
	              + inZAxisA.s[3] * inWAxisB.s[2];

	outZAxis.s[3] = inZAxisA.s[0] * inXAxisB.s[3]
	              + inZAxisA.s[1] * inYAxisB.s[3]
	              + inZAxisA.s[2] * inZAxisB.s[3]
	              + inZAxisA.s[3] * inWAxisB.s[3];
				  
	// W Axis
	outWAxis.s[0] = inWAxisA.s[0] * inXAxisB.s[0]
	              + inWAxisA.s[1] * inYAxisB.s[0]
	              + inWAxisA.s[2] * inZAxisB.s[0]
	              + inWAxisA.s[3] * inWAxisB.s[0];

	outWAxis.s[1] = inWAxisA.s[0] * inXAxisB.s[1]
	              + inWAxisA.s[1] * inYAxisB.s[1]
	              + inWAxisA.s[2] * inZAxisB.s[1]
	              + inWAxisA.s[3] * inWAxisB.s[1];

	outWAxis.s[2] = inWAxisA.s[0] * inXAxisB.s[2]
	              + inWAxisA.s[1] * inYAxisB.s[2]
	              + inWAxisA.s[2] * inZAxisB.s[2]
	              + inWAxisA.s[3] * inWAxisB.s[2];

	outWAxis.s[3] = inWAxisA.s[0] * inXAxisB.s[3]
	              + inWAxisA.s[1] * inYAxisB.s[3]
	              + inWAxisA.s[2] * inZAxisB.s[3]
	              + inWAxisA.s[3] * inWAxisB.s[3];
}

//-----------------------------------------------------------------------------
inline void TransformMatrixByMatrix (
	cl_float4 &xAxisA,
	cl_float4 &yAxisA,
	cl_float4 &zAxisA,
	cl_float4 &wAxisA,
	const cl_float4 &inXAxisB,
	const cl_float4 &inYAxisB,
	const cl_float4 &inZAxisB,
	const cl_float4 &inWAxisB)
{
	cl_float4 tempXAxis;
	cl_float4 tempYAxis;
	cl_float4 tempZAxis;
	cl_float4 tempWAxis;

	TransformMatrixByMatrix(
		tempXAxis,
		tempYAxis,
		tempZAxis,
		tempWAxis,
		xAxisA,
		yAxisA,
		zAxisA,
		wAxisA,
		inXAxisB,
		inYAxisB,
		inZAxisB,
		inWAxisB);

	xAxisA = tempXAxis;
	yAxisA = tempYAxis;
	zAxisA = tempZAxis;
	wAxisA = tempWAxis;
}