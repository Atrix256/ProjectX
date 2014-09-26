/*==================================================================================================

Systems.cpp

The code for "Systems", which are part of the ECS Entity-Component-Systems model.

==================================================================================================*/

#include "Systems.h"
#include "Components.h"
#include "Game/MatrixMath.h"
#include "ECS.h"

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
void CECSSystemFPSCamera::UpdateEntity (float elapsedSeconds, const CECSComponentInput &input, CECSComponentCamera &camera)
{
	// mouse X changes yaw, mouse Y changes pitch.
	const float m_mouseSensitivity = 0.01f;
	camera.m_yaw += input.m_mouseMoveX * m_mouseSensitivity;
	camera.m_pitch += input.m_mouseMoveY * -m_mouseSensitivity;

	// clamp pitch to reasonable values
	if (camera.m_pitch > camera.m_pitchMax)
		camera.m_pitch = camera.m_pitchMax;
	else if (camera.m_pitch < camera.m_pitchMin)
		camera.m_pitch = camera.m_pitchMin;
}

//--------------------------------------------------------------------------------------------------
void CECSSystemPlayerController::UpdateEntity (
	float elapsedSeconds,
	const CECSComponentCamera &camera,
	CECSComponentPhysics &physics,
	CECSComponentInput &input
)
{
	// reset mouse each frame, probably should be in an "input" system instead of here
	input.m_mouseMoveX = 0.0f;
	input.m_mouseMoveY = 0.0f;

	// make sure the physics position delta is cleared to zero
	physics.m_positionDelta[0] = 0.0f;
	physics.m_positionDelta[1] = 0.0f;
	physics.m_positionDelta[2] = 0.0f;

	// get camera orientation
	float3 xAxis, yAxis, zAxis;
	Camera_GetBasisVectors(camera, xAxis, yAxis, zAxis);

	// basic movement
	const float moveSpeed = input.m_keySprint ? 20.0f : 5.0f;
	if (input.m_keyWalkForward)
		physics.m_positionDelta += zAxis * elapsedSeconds * moveSpeed;

	if (input.m_keyWalkBack)
		physics.m_positionDelta -= zAxis * elapsedSeconds * moveSpeed;

	if (input.m_keyWalkLeft)
		physics.m_positionDelta += xAxis * elapsedSeconds * moveSpeed;

	if (input.m_keyWalkRight)
		physics.m_positionDelta -= xAxis * elapsedSeconds * moveSpeed;
}