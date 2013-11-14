/*==================================================================================================

CGame.h

The root class for game logic

==================================================================================================*/

#pragma once

#include "CPlayer.h"

class CGame
{
public:
	static void Update(float elapsed);
	static void SetPlayerPos(float X, float Y, float Z);
	static void SetPlayerFacing(float angle);

private:
	static CPlayer m_player;
};