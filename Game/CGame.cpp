/*==================================================================================================

CGame.cpp

The root class for game logic

==================================================================================================*/

#include "CGame.h"
#include "ECS\ECS.h"

CPlayer CGame::m_player;
float CGame::m_timeBucket = 0.0f;
const float CGame::c_gameLogicInterval = 1.0f / 60.0f;
SData_GameData CGame::m_gameData;

//--------------------------------------------------------------------------------------------------
void CGame::Update (float elapsed)
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

	ECS::Update(elapsed);
	m_player.Update(elapsed);
}

//--------------------------------------------------------------------------------------------------
void CGame::SetPlayerPos (float X, float Y, float Z)
{
	m_player.SetPosition(X, Y, Z);
}

//--------------------------------------------------------------------------------------------------
void CGame::SetPlayerFacing (float X, float Y, float Z)
{
	m_player.SetFacing(X, Y, Z);
}

//--------------------------------------------------------------------------------------------------
void CGame::LoadGameData ()
{
	//TODO: if game data file doesn't exist, save it out so there is one!
	//NOTE: can't just check for failure of load, since we don't want to stomp whatever changes the person is making
	//      just because of a typo
	DataSchemasXML::Load(m_gameData, "./data/gamedata.xml", "GameData");

	// Create the player entity
	unsigned int entityIndex = SData::GetEntryById(m_gameData.m_Entity, m_gameData.m_PlayerEntity, -1);
	// todo: log an error or something...
	Assert_(entityIndex != -1);
	if (entityIndex != -1)
	{
		const SData_ECSEntity &entity = m_gameData.m_Entity[entityIndex];

		// get the components that should be installed
		#define ComponentBegin(name, hint) \
			unsigned int componentIndex##name = SData::GetEntryById( \
				m_gameData.m_Component##name, \
				entity.m_Component##name, \
				-1); \
			SData_Component##name *component##name = componentIndex##name != -1 \
				? &m_gameData.m_Component##name[componentIndex##name] \
				: NULL;
		#include "ECS\ComponentList.h"

		// make flags for which systems should be installed
		unsigned int systemFlags = ECS::e_systemFlagNone;
		#define SystemBegin(name, single, hint) \
			if (entity.m_System##name) \
				systemFlags |= ECS::e_systemFlag##name;
		#include "ECS\SystemList.h"

		// create the entity
		ECS::CreateEntity(systemFlags
		#define ComponentBegin(name, hint) , component##name
		#include "ECS\ComponentList.h"
		);
	}
}