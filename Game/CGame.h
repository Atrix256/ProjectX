/*==================================================================================================

CGame.h

The root class for game logic

==================================================================================================*/

#pragma once

#include "CPlayer.h"
#include "DataSchemas/DataSchemasXML.h"

class CGame
{
public:
	static void Init () { LoadGameData(); m_player.Init(); }

	static void Update (float elapsed);

	static void LoadGameData ();

	static const SData_GameData& GameData () { return m_gameData; }

private:
	static CPlayer m_player;
	static float m_timeBucket;
	static SData_GameData m_gameData;

	static const float c_gameLogicInterval;
};