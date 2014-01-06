/*==================================================================================================

SharedGeometry.h

Structs shared by both kernel and host code

==================================================================================================*/

#pragma once

#include "SharedTypes.h"

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
	cl_float m_normalTextureIndex;
	cl_float m_emissiveTextureIndex;
	cl_float m_pad2;
	cl_float m_pad3;
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

struct SPlane
{
	cl_float4 m_equation;
	float3 m_UAxis;
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

#define SSECTOR_NUMPLANES 6

struct SSectorPlane
{
	cl_float4 m_equation;
	float3 m_UAxis;
	cl_float2 m_textureScale;
	cl_uint m_materialIndex;
	cl_uint m_portalNextSector;
	cl_float4 m_portalWindow;
	TObjectId m_objectId;
	cl_uint m_pack1a;
	cl_uint m_pack1b;
	cl_uint m_pack1c;
};

struct SSector
{
	struct SSectorPlane m_planes[SSECTOR_NUMPLANES];
	cl_uint m_castsShadows;
	cl_uint m_pack1a;
	cl_uint m_pack1b;
	cl_uint m_pack1c;
};

struct SPointLight
{
	float3 m_position;
	float3 m_color;
};
