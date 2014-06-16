/*==================================================================================================

ECS.cpp

The entry point for the ECS system

==================================================================================================*/

#include "ECS.h"
#include "Systems.h"
#include "Components.h"
#include "ECSEnums.h"

namespace ECS
{
	static unsigned int g_nextEntityId = 1;

	//--------------------------------------------------------------------------------------------------
	void CreateEntity (
		unsigned int systems
		#define ComponentBegin(name, hint) , const SData_Component##name *componentData##name
		#include "ECS\ComponentList.h"
	)
	{
		// reserve an entity id
		unsigned int entityId = g_nextEntityId;
		g_nextEntityId++;

		// create the components for the entity
		#define ComponentBegin(name, hint) \
		CECSComponent##name *component##name = componentData##name \
			? CECSComponent##name::Create(entityId, *componentData##name) \
			: NULL;
		#include "ECS\ComponentList.h"

		// Register this entity with the systems it wants to register with
		#define SystemBegin(name, single, hint) \
		if (systems & e_systemFlag##name) \
			CECSSystem##name::Register(entityId);
		#include "ECS\SystemList.h"
	}

	//--------------------------------------------------------------------------------------------------
	void Update (float elapsedSeconds)
	{
		// update each system
		#define SystemBegin(name, single, hint) CECSSystem##name::Update(elapsedSeconds);
		#include "SystemList.h"
	}
};