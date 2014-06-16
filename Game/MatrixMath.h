/*==================================================================================================

MatrixMath.h

Matrix math routines used by host code

==================================================================================================*/

#pragma once

#include "Platform/float3.h"

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
inline void TransformMatrixByMatrix (
	cl_float4 &xAxisA,
	cl_float4 &yAxisA,
	cl_float4 &zAxisA,
	cl_float4 &wAxisA,
	const cl_float4 &inXAxisB,
	const cl_float4 &inYAxisB,
	const cl_float4 &inZAxisB,
	const cl_float4 &inWAxisB);

//-----------------------------------------------------------------------------
inline void TransformMatrixByMatrix (
	float3 &xAxisA,
	float3 &yAxisA,
	float3 &zAxisA,
	const float3 &inXAxisB,
	const float3 &inYAxisB,
	const float3 &inZAxisB);

//-----------------------------------------------------------------------------
// FUNCTIONS
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
inline void MatrixRotationX (
	cl_float4 &xAxis,
	cl_float4 &yAxis,
	cl_float4 &zAxis,
	cl_float4 &wAxis,
	const float rot)
{
	xAxis.s[0] = 1.0f;
	xAxis.s[1] = 0.0f;
	xAxis.s[2] = 0.0f;
	xAxis.s[3] = 0.0f;

	yAxis.s[0] = 0.0f;
	yAxis.s[1] = cos(rot);
	yAxis.s[2] = -sin(rot);
	yAxis.s[3] = 0.0f;

	zAxis.s[0] = 0.0f;
	zAxis.s[1] = sin(rot);
	zAxis.s[2] = cos(rot);
	zAxis.s[3] = 0.0f;

	wAxis.s[0] = 0.0f;
	wAxis.s[1] = 0.0f;
	wAxis.s[2] = 0.0f;
	wAxis.s[3] = 1.0f;
}

//-----------------------------------------------------------------------------
inline void MatrixRotationX (
	float3 &xAxis,
	float3 &yAxis,
	float3 &zAxis,
	const float rot)
{
	xAxis[0] = 1.0f;
	xAxis[1] = 0.0f;
	xAxis[2] = 0.0f;

	yAxis[0] = 0.0f;
	yAxis[1] = cos(rot);
	yAxis[2] = -sin(rot);

	zAxis[0] = 0.0f;
	zAxis[1] = sin(rot);
	zAxis[2] = cos(rot);
}

//-----------------------------------------------------------------------------
inline void MatrixRotationY (
	cl_float4 &xAxis,
	cl_float4 &yAxis,
	cl_float4 &zAxis,
	cl_float4 &wAxis,
	const float rot)
{
	xAxis.s[0] = cos(rot);
	xAxis.s[1] = 0.0f;
	xAxis.s[2] = sin(rot);
	xAxis.s[3] = 0.0f;

	yAxis.s[0] = 0.0f;
	yAxis.s[1] = 1.0f;
	yAxis.s[2] = 0.0f;
	yAxis.s[3] = 0.0f;

	zAxis.s[0] = -sin(rot);
	zAxis.s[1] = 0.0f;
	zAxis.s[2] = cos(rot);
	zAxis.s[3] = 0.0f;

	wAxis.s[0] = 0.0f;
	wAxis.s[1] = 0.0f;
	wAxis.s[2] = 0.0f;
	wAxis.s[3] = 1.0f;
}

//-----------------------------------------------------------------------------
inline void MatrixRotationY (
	float3 &xAxis,
	float3 &yAxis,
	float3 &zAxis,
	const float rot)
{
	xAxis[0] = cos(rot);
	xAxis[1] = 0.0f;
	xAxis[2] = sin(rot);

	yAxis[0] = 0.0f;
	yAxis[1] = 1.0f;
	yAxis[2] = 0.0f;

	zAxis[0] = -sin(rot);
	zAxis[1] = 0.0f;
	zAxis[2] = cos(rot);
}

//-----------------------------------------------------------------------------
inline void MatrixRotationZ (
	cl_float4 &xAxis,
	cl_float4 &yAxis,
	cl_float4 &zAxis,
	cl_float4 &wAxis,
	const float rot)
{
	xAxis.s[0] = cos(rot);
	xAxis.s[1] = -sin(rot);
	xAxis.s[2] = 0.0f;
	xAxis.s[3] = 0.0f;

	yAxis.s[0] = sin(rot);
	yAxis.s[1] = cos(rot);
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
inline void MatrixRotationZ (
	float3 &xAxis,
	float3 &yAxis,
	float3 &zAxis,
	const float rot)
{
	xAxis[0] = cos(rot);
	xAxis[1] = -sin(rot);
	xAxis[2] = 0.0f;

	yAxis[0] = sin(rot);
	yAxis[1] = cos(rot);
	yAxis[2] = 0.0f;

	zAxis[0] = 0.0f;
	zAxis[1] = 0.0f;
	zAxis[2] = 1.0f;
}

//-----------------------------------------------------------------------------
inline void MatrixRotation (
	cl_float4 &xAxis,
	cl_float4 &yAxis,
	cl_float4 &zAxis,
	cl_float4 &wAxis,
	const float rotX,
	const float rotY,
	const float rotZ)
{
	// make X axis rotation
	MatrixRotationX(xAxis, yAxis, zAxis, wAxis, rotX);

	// apply Y axis rotation
	cl_float4 tempX, tempY, tempZ, tempW;
	MatrixRotationY(tempX, tempY, tempZ, tempW, rotY);
	TransformMatrixByMatrix(xAxis, yAxis, zAxis, wAxis, tempX, tempY, tempZ, tempW);

	// apply Z axis rotation
	MatrixRotationZ(tempX, tempY, tempZ, tempW, rotZ);
	TransformMatrixByMatrix(xAxis, yAxis, zAxis, wAxis, tempX, tempY, tempZ, tempW);
}

//-----------------------------------------------------------------------------
inline void MatrixRotation (
	float3 &xAxis,
	float3 &yAxis,
	float3 &zAxis,
	const float rotX,
	const float rotY,
	const float rotZ)
{
	// make X axis rotation
	MatrixRotationX(xAxis, yAxis, zAxis, rotX);

	// apply Y axis rotation
	float3 tempX, tempY, tempZ;
	MatrixRotationY(tempX, tempY, tempZ, rotY);
	TransformMatrixByMatrix(xAxis, yAxis, zAxis, tempX, tempY, tempZ);

	// apply Z axis rotation
	MatrixRotationZ(tempX, tempY, tempZ, rotZ);
	TransformMatrixByMatrix(xAxis, yAxis, zAxis, tempX, tempY, tempZ);
}

//-----------------------------------------------------------------------------
inline void MatrixUnrotation (
	cl_float4 &xAxis,
	cl_float4 &yAxis,
	cl_float4 &zAxis,
	cl_float4 &wAxis,
	const float rotX,
	const float rotY,
	const float rotZ)
{
	// make negative Z axis rotation
	MatrixRotationZ(xAxis, yAxis, zAxis, wAxis, -rotZ);

	// apply negative Y axis rotation
	cl_float4 tempX, tempY, tempZ, tempW;
	MatrixRotationY(tempX, tempY, tempZ, tempW, -rotY);
	TransformMatrixByMatrix(xAxis, yAxis, zAxis, wAxis, tempX, tempY, tempZ, tempW);

	// apply negative X axis rotation
	MatrixRotationX(tempX, tempY, tempZ, tempW, -rotX);
	TransformMatrixByMatrix(xAxis, yAxis, zAxis, wAxis, tempX, tempY, tempZ, tempW);
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
	float3 &outXAxis,
	float3 &outYAxis,
	float3 &outZAxis,
	const float3 &inXAxisA,
	const float3 &inYAxisA,
	const float3 &inZAxisA,
	const float3 &inXAxisB,
	const float3 &inYAxisB,
	const float3 &inZAxisB)
{
	// X Axis
	outXAxis[0] = inXAxisA[0] * inXAxisB[0]
	            + inXAxisA[1] * inYAxisB[0]
	            + inXAxisA[2] * inZAxisB[0];

	outXAxis[1] = inXAxisA[0] * inXAxisB[1]
	            + inXAxisA[1] * inYAxisB[1]
	            + inXAxisA[2] * inZAxisB[1];

	outXAxis[2] = inXAxisA[0] * inXAxisB[2]
	            + inXAxisA[1] * inYAxisB[2]
	            + inXAxisA[2] * inZAxisB[2];

	// Y Axis
	outYAxis[0] = inYAxisA[0] * inXAxisB[0]
	            + inYAxisA[1] * inYAxisB[0]
	            + inYAxisA[2] * inZAxisB[0];

	outYAxis[1] = inYAxisA[0] * inXAxisB[1]
	            + inYAxisA[1] * inYAxisB[1]
	            + inYAxisA[2] * inZAxisB[1];

	outYAxis[2] = inYAxisA[0] * inXAxisB[2]
	            + inYAxisA[1] * inYAxisB[2]
	            + inYAxisA[2] * inZAxisB[2];

	// Z Axis
	outZAxis[0] = inZAxisA[0] * inXAxisB[0]
	            + inZAxisA[1] * inYAxisB[0]
	            + inZAxisA[2] * inZAxisB[0];

	outZAxis[1] = inZAxisA[0] * inXAxisB[1]
	            + inZAxisA[1] * inYAxisB[1]
	            + inZAxisA[2] * inZAxisB[1];

	outZAxis[2] = inZAxisA[0] * inXAxisB[2]
	            + inZAxisA[1] * inYAxisB[2]
	            + inZAxisA[2] * inZAxisB[2];
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

//-----------------------------------------------------------------------------
inline void TransformMatrixByMatrix (
	float3 &xAxisA,
	float3 &yAxisA,
	float3 &zAxisA,
	const float3 &inXAxisB,
	const float3 &inYAxisB,
	const float3 &inZAxisB)
{
	float3 tempXAxis;
	float3 tempYAxis;
	float3 tempZAxis;

	TransformMatrixByMatrix(
		tempXAxis,
		tempYAxis,
		tempZAxis,
		xAxisA,
		yAxisA,
		zAxisA,
		inXAxisB,
		inYAxisB,
		inZAxisB);

	xAxisA = tempXAxis;
	yAxisA = tempYAxis;
	zAxisA = tempZAxis;
}

inline float DegreesToRadians(float degrees)
{
	return (degrees * 3.14159f) / 180.0f;
}