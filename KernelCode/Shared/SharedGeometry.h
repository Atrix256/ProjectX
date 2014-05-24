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
	cl_float m_pad1;
	cl_float m_pad2;

	float3 m_absorbance;
};

struct SPortal
{
	cl_float4 m_xaxis;
	cl_float4 m_yaxis;
	cl_float4 m_zaxis;
	cl_float4 m_waxis;
	float3    m_position;

	unsigned int m_sector;
	unsigned int m_setPosition;
	unsigned int m_pad[2];
};

struct SSphere
{
	cl_float4 m_positionAndRadius;
	cl_float2 m_textureScale;
	cl_float2 m_textureOffset;
	cl_uint m_materialIndex;
	cl_uint m_portalIndex;
	TObjectId m_objectId;
	cl_uchar m_castsShadows;
	cl_uchar m_pack1b;
	cl_uchar m_pack1c;
	cl_uchar m_pack1d;
};

// flags for each 3d half space
enum EHalfSpaceFlags
{
	e_halfSpacePosY = 0x01,
	e_halfSpaceNegY = 0x02,
};

struct SModelTriangle
{
	cl_float4 m_plane;
	cl_float4 m_planeBC;
	cl_float4 m_planeCA;
	cl_float2 m_textureA;
	cl_float2 m_textureB;

	cl_float2 m_textureC;
	TObjectId m_objectId;
	cl_uint m_halfSpaceFlags;  // could be an 8 bit number, just need 6 bits currently
	float3 m_tangent;
	float3 m_bitangent;
	cl_float4 m_pack2;
};

struct SModelObject
{
	cl_uint m_startTriangleIndex;    // this is where the triangles start - also the beginning of the e_halfSpaceNegY triangles
	cl_uint m_mixStartTriangleIndex; // this is where the triangles start that are both e_halfSpaceNegY and e_halfSpacePosY
	cl_uint m_mixStopTriangleIndex;  // this is where the triangles end that are both e_halfSpaceNegY and e_halfSpacePosY - also the start of the e_halfSpacePosY triangles
	cl_uint m_stopTriangleIndex;     // this is where the triangles end - also the end of e_halfSpacePosY triangles

	cl_uint m_castsShadows;
	cl_uint m_materialIndex;
	cl_uint m_portalIndex;
	cl_uint m_pack3;
};

struct SModelInstance
{
	cl_uint m_startObjectIndex;
	cl_uint m_stopObjectIndex;
	cl_uint m_pack1;
	cl_uint m_pack2;

	cl_float4 m_boundingSphere;

	cl_float4 m_transform1;
	cl_float4 m_transform2;
	cl_float4 m_transform3;
	cl_float4 m_transform4;
};

#define SSECTOR_NUMPLANES 6

struct SSectorPlane
{
	float3 m_UAxis;
	cl_float2 m_textureScale;
	cl_float2 m_textureOffset;
	cl_float4 m_portalWindow;
	TObjectId m_objectId;
	cl_uint m_pack1a;
	cl_uint m_materialIndex;
	cl_uint m_portalIndex;
};

struct SSector
{
	struct SSectorPlane m_planes[SSECTOR_NUMPLANES];
	
	float3 m_halfDims;

	float3 m_ambientLight;
	
	cl_uint m_castsShadows;
	cl_uint m_staticSphereStartIndex;
	cl_uint m_staticSphereStopIndex;
	cl_uint m_pack1;

	cl_uint m_staticLightStartIndex;
	cl_uint m_staticLightStopIndex;
	cl_uint m_staticModelStartIndex;
	cl_uint m_staticModelStopIndex;
};

struct SPointLight
{
	// point light params
	float3 m_position;
	float3 m_color;
	float3 m_attenuationConstDistDistsq;
	
	// spot light params
	float3 m_spotLightReverseDir;

	float m_spotLightcosThetaOver2;
	float m_spotLightcosPhiOver2;
	float m_spotLightFalloffFactor;
	float m_pad;
};