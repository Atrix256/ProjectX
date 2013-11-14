/*==================================================================================================

CPlayer.h

Holds info about the player and also handles player input

==================================================================================================*/

#pragma once

#include "Platform/float3.h"

class CPlayer
{
public:
	void Update (float elapsed);
	void SetPosition(float X, float Y, float Z);
	void SetFacing(float angle);

private:
	float3 m_position;
};