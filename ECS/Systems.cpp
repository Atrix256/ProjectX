/*==================================================================================================

Systems.cpp

The code for "Systems", which are part of the ECS Entity-Component-Systems model.

==================================================================================================*/

#include "Systems.h"
#include "Components.h"
#include "Game/MatrixMath.h"

// make the static regsitered entity lists for each system
#define SystemBegin(name, single, hint) \
	std::vector<unsigned int> CECSSystem##name::s_registeredEntities;
#include "SystemList.h"

//--------------------------------------------------------------------------------------------------
// make the system scope update function for each system.  This calls Update() for each entity
// registered, passing the elapsed time and all the necesary components, with the proper access
// rules.
#define SystemBegin(name, single, hint) \
void CECSSystem##name::UpdateSystem (float elapsedSeconds) \
{ \
	for (std::vector<unsigned int>::iterator it = s_registeredEntities.begin(); it != s_registeredEntities.end(); ++it) \
	{ \
		CECSSystem##name::UpdateEntity(elapsedSeconds

#define SystemComponent(type, access, hint) \
		, CECSComponent##type::MustGetByEntityId(*it)

#define SystemEnd \
		); \
	} \
}
#include "SystemList.h"

//--------------------------------------------------------------------------------------------------
void CECSSystemCamera::UpdateEntity (float elapsedSeconds)
{
	// this function isn't really needed
	int ijkl = 0;
}

//--------------------------------------------------------------------------------------------------
void CECSSystemPlayerController::UpdateEntity (float elapsedSeconds, CECSComponentInput &input, CECSComponentBearings &bearings)
{
	// handle mouse movement changing the entities rotation
	const float m_mouseSensitivity = 0.01f;

	// consume X mouse movement
	if (input.m_mouseMoveX != 0.0f)
	{
		float3 xAxis, yAxis, zAxis;
		MatrixRotationY(xAxis, yAxis, zAxis, input.m_mouseMoveX * m_mouseSensitivity * 1.0f);
		TransformMatrixByMatrix(
			bearings.m_rotationAxisX, bearings.m_rotationAxisY, bearings.m_rotationAxisZ,
			xAxis, yAxis,zAxis,
			bearings.m_rotationAxisX, bearings.m_rotationAxisY, bearings.m_rotationAxisZ);
		input.m_mouseMoveX = 0.0f;

		// make sure rotation is normalized
		bearings.m_rotationAxisX = normalize(bearings.m_rotationAxisX);
		bearings.m_rotationAxisY = normalize(bearings.m_rotationAxisY);
		bearings.m_rotationAxisZ = normalize(bearings.m_rotationAxisZ);
	}

	// consume Y mouse movement
	if (input.m_mouseMoveY!= 0.0f)
	{
		float3 xAxis, yAxis, zAxis;
		MatrixRotationX(xAxis, yAxis, zAxis, input.m_mouseMoveY * m_mouseSensitivity * -1.0f);
		TransformMatrixByMatrix(
			bearings.m_rotationAxisX, bearings.m_rotationAxisY, bearings.m_rotationAxisZ,
			xAxis, yAxis,zAxis,
			bearings.m_rotationAxisX, bearings.m_rotationAxisY, bearings.m_rotationAxisZ);
		input.m_mouseMoveY = 0.0f;

		// make sure rotation is normalized
		bearings.m_rotationAxisX = normalize(bearings.m_rotationAxisX);
		bearings.m_rotationAxisY = normalize(bearings.m_rotationAxisY);
		bearings.m_rotationAxisZ = normalize(bearings.m_rotationAxisZ);
	}

	// basic movement
	const float moveSpeed = input.m_keySprint ? 20.0f : 5.0f;
	if (input.m_keyWalkForward)
		bearings.m_position += bearings.m_rotationAxisZ * elapsedSeconds * moveSpeed;

	if (input.m_keyWalkBack)
		bearings.m_position -= bearings.m_rotationAxisZ * elapsedSeconds * moveSpeed;

	if (input.m_keyWalkLeft)
		bearings.m_position += bearings.m_rotationAxisX * elapsedSeconds * moveSpeed;

	if (input.m_keyWalkRight)
		bearings.m_position -= bearings.m_rotationAxisX * elapsedSeconds * moveSpeed;
}

//--------------------------------------------------------------------------------------------------
void CECSSystemCreaturePhysics::UpdateEntity (float elapsedSeconds, CECSComponentBearings &bearings)
{
	// Physics!
	int ijkl = 0;
}
