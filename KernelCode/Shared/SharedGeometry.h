/*==================================================================================================

SharedGeometry.h

Structs shared by both kernel and host code

==================================================================================================*/

#pragma once

#ifndef OPENCL
	#include "Platform/float3.h"
#else
	typedef unsigned int cl_uint;
	typedef float cl_float;
	typedef float2 cl_float2;
	typedef float4 cl_float4;
	typedef unsigned char cl_uchar;
#endif

#define c_invalidObjectId 0
typedef cl_uint TObjectId;

struct SMaterial
{
	float3 m_diffuseColor;
	cl_float4 m_specularColorAndPower;
	float3 m_emissiveColor;
	cl_float m_reflectionAmount;
	cl_float m_refractionIndex;
	cl_float m_refractionAmount;
	cl_float m_diffuseTextureIndex;
};

struct SSphere
{
	cl_float4 m_positionAndRadius;
	cl_float2 m_textureScale;
	cl_float2 m_pack3;
	cl_uint m_materialIndex;
	TObjectId m_objectId;
	cl_uchar m_castsShadows;
	cl_uchar m_pack1b;
	cl_uchar m_pack1c;
	cl_uchar m_pack1d;
	cl_uint m_pack2;
};

struct SAABox
{
	float3 m_position;
	float3 m_scale;
	cl_float2 m_textureScale;
	cl_float2 m_pack3;
	cl_uint m_materialIndex;
	TObjectId m_objectId;
	cl_uchar m_castsShadows;
	cl_uchar m_pack1b;
	cl_uchar m_pack1c;
	cl_uchar m_pack1d;
	cl_uint m_pack2;
};

struct SPointLight
{
	float3 m_position;
	float3 m_color;
};
