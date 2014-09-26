/*==================================================================================================

Components.cpp

The code for "Components", which are part of the ECS Entity-Component-Systems model.  Components
hold data for a specific entity and have no logic on them.  They are read and written too by various
systems to do work on entities.

==================================================================================================*/

#include "Components.h"
#include "DataSchemas\DataSchemasStructs.h"
#include "Game\MatrixMath.h"
#include "Platform\CDirectX.h"

// define the static member component lists
#define ComponentBegin(name, hint) \
	CECSComponent##name::TList CECSComponent##name::s_components;
#include "ComponentList.h"

//--------------------------------------------------------------------------------------------------
CECSComponentBearings::CECSComponentBearings (
	unsigned int entityId,
	const struct SData_ComponentBearings &data
)
{
	m_entityId = entityId;

	m_sector = CDirectX::GetWorld().GetSectorIDByName(data.m_Sector.c_str());

	m_position[0] = data.m_Position.m_x;
	m_position[1] = data.m_Position.m_y;
	m_position[2] = data.m_Position.m_z;
}

//--------------------------------------------------------------------------------------------------
CECSComponentCamera::CECSComponentCamera (
	unsigned int entityId,
	const struct SData_ComponentCamera &data
)
	: m_pitchMax(DegreesToRadians(data.m_PitchMax))
	, m_pitchMin(DegreesToRadians(data.m_PitchMin))
{
	m_entityId = entityId;
	m_pitch = DegreesToRadians(data.m_Pitch);
	m_yaw = DegreesToRadians(data.m_Yaw);
}

//--------------------------------------------------------------------------------------------------
CECSComponentInput::CECSComponentInput (
	unsigned int entityId,
	const struct SData_ComponentInput &data
)
{
	m_entityId = entityId;
	m_mouseMoveX = m_mouseMoveY = 0.0f;

	#define INPUT_TOGGLE(name, resetOnKeyUp) bool m_key##name = false;
	#include "Game/InputToggleList.h"
}

//--------------------------------------------------------------------------------------------------
CECSComponentPhysics::CECSComponentPhysics (
	unsigned int entityId,
	const struct SData_ComponentPhysics &data
)
{
	m_entityId = entityId;
	m_cylinderHalfDims[0] = m_cylinderHalfDims[2]  = data.m_CylinderRadius;
	m_cylinderHalfDims[1] = data.m_CylinderHeight / 2.0f;
}

//--------------------------------------------------------------------------------------------------
void Camera_GetBasisVectors (const CECSComponentCamera &camera, float3 &xAxis, float3 &yAxis, float3 &zAxis)
{
	const float theta = camera.m_yaw;
	const float phi = camera.m_pitch;
	zAxis[0] = cos(theta) * cos(phi);
	zAxis[1] = sin(phi);
	zAxis[2] = sin(theta) * cos(phi);

	const float3 trueUp = {0.0f,1.0f,0.0f};
	xAxis = normalize(cross(trueUp, zAxis));
	yAxis = normalize(cross(zAxis, xAxis));
}