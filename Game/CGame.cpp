/*==================================================================================================

CGame.cpp

The root class for game logic

==================================================================================================*/

#include "CGame.h"

CPlayer CGame::m_player;
float CGame::m_timeBucket = 0.0f;
const float CGame::c_gameLogicInterval = 1.0f / 60.0f;

//--------------------------------------------------------------------------------------------------
void CGame::Update(float elapsed)
{
	// fixed logic interval makes the game feel choppy
	/*
	m_timeBucket += elapsed;

	while (m_timeBucket > c_gameLogicInterval)
	{
		m_player.Update(c_gameLogicInterval);
		m_timeBucket -= c_gameLogicInterval;
	}
	*/

	m_player.Update(elapsed);
}

//--------------------------------------------------------------------------------------------------
void CGame::SetPlayerPos(float X, float Y, float Z)
{
	m_player.SetPosition(X, Y, Z);
}

//--------------------------------------------------------------------------------------------------
void CGame::SetPlayerFacing(float angle)
{
	m_player.SetFacing(angle);
}