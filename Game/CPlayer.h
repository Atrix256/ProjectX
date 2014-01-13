/*==================================================================================================

CPlayer.h

Holds info about the player and also handles player input

==================================================================================================*/

#pragma once

#include "Platform/float3.h"

class CPlayer
{
public:
	CPlayer();

	void Init ();

	void Update (float elapsed);
	void SetPosition (float X, float Y, float Z);
	void SetFacing (float angle);

private:
	float	m_playerHeight;
	float3	m_position;
	float3	m_velocity;
	bool	m_onGround;
	bool	m_crouched;
};