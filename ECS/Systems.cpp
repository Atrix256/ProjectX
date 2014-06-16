/*==================================================================================================

Systems.cpp

The code for "Systems", which are part of the ECS Entity-Component-Systems model.

==================================================================================================*/

#include "Systems.h"
#include "Components.h"

// make the static regsitered entity lists for each system
#define SystemBegin(name, single, hint) \
	std::vector<unsigned int> CECSSystem##name::s_registeredEntities;
#include "SystemList.h"

//--------------------------------------------------------------------------------------------------
// make the system scope update function for each system.  This calls Update() for each entity
// registered, passing the elapsed time and all the necesary components, with the proper access
// rules.
#define SystemBegin(name, single, hint) \
void CECSSystem##name::Update (float elapsedSeconds) \
{ \
	for (std::vector<unsigned int>::iterator it = s_registeredEntities.begin(); it != s_registeredEntities.end(); ++it) \
	{ \
		CECSSystem##name::Update(elapsedSeconds

#define SystemComponent(type, access, hint) \
		, CECSComponent##type::MustGetByEntityId(*it)

#define SystemEnd \
		); \
	} \
}
#include "SystemList.h"

//--------------------------------------------------------------------------------------------------
void CECSSystemCamera::Update (float elapsedSeconds, const CECSComponentBearings &bearings)
{
	int ijkl = 0;
}

//--------------------------------------------------------------------------------------------------
void CECSSystemPlayerController::Update (float elapsedSeconds, CECSComponentBearings &bearings)
{
	int ijkl = 0;
}

//--------------------------------------------------------------------------------------------------
void CECSSystemCreaturePhysics::Update (float elapsedSeconds, CECSComponentBearings &bearings)
{
	int ijkl = 0;
}