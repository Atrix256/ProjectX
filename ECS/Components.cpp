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

	MatrixRotation(
		m_rotationAxisX,
		m_rotationAxisY,
		m_rotationAxisZ,
		DegreesToRadians(data.m_Rotation.m_x),
		DegreesToRadians(data.m_Rotation.m_y),
		DegreesToRadians(data.m_Rotation.m_z)
	);

	m_quaternion.s[0] = 0.0f;
	m_quaternion.s[1] = 0.0f;
	m_quaternion.s[2] = 0.0f;
	m_quaternion.s[3] = 1.0f;
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