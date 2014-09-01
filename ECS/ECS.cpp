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
	static bool g_doingUpdate = false;

	//--------------------------------------------------------------------------------------------------
	void CreateEntity (
		unsigned int systems
		#define ComponentBegin(name, hint) , const struct SData_Component##name *componentData##name
		#include "ECS\ComponentList.h"
	)
	{
		// reserve an entity id
		unsigned int entityId = g_nextEntityId++;

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
		g_doingUpdate = true;

		// update each system - this can be threaded in the future
		#define SystemBegin(name, single, hint) CECSSystem##name::UpdateSystem(elapsedSeconds);
		#include "SystemList.h"

		g_doingUpdate = false;
	}

	//--------------------------------------------------------------------------------------------------
	void OnMouseMove (float relX, float relY)
	{
		// We access components directly without regard for threading matters, so we better not do this
		// during an update call!
		Assert_(g_doingUpdate == false);

		// apply this mouse movement to all input components
		CECSComponentInput::TList& list = CECSComponentInput::All();
		for (CECSComponentInput::TList::iterator it = list.begin(); it != list.end(); ++it)
		{
			CECSComponentInput *component = *it;
			if (!component)
				continue;

			component->m_mouseMoveX += relX;
			component->m_mouseMoveY += relY;
		}
	}

	//--------------------------------------------------------------------------------------------------
	void SetInputState(
#define INPUT_TOGGLE(name, resetOnKeyUp) bool key##name,
	#include "Game/InputToggleList.h"
	void *dummy)
	{
		// We access components directly without regard for threading matters, so we better not do this
		// during an update call!
		Assert_(g_doingUpdate == false);

		// apply this mouse movement to all input components
		CECSComponentInput::TList& list = CECSComponentInput::All();
		for (CECSComponentInput::TList::iterator it = list.begin(); it != list.end(); ++it)
		{
			CECSComponentInput *component = *it;
			if (!component)
				continue;

			#define INPUT_TOGGLE(name, resetOnKeyUp) component->m_key##name = key##name;
			#include "Game/InputToggleList.h"
		}
	}

	//--------------------------------------------------------------------------------------------------
	bool GetCameraTransform (
		float3 &pos,
		float3 &fwd,
		float3 &up,
		float3 &left,
		cl_uint &sector)
	{
		// Get the bearings from the bearings component of the single entity registered with the camera,
		// if there is one registered.

		if (CECSSystemCamera::GetRegisteredEntityCount() == 0)
			return false;

		unsigned int cameraEntity = CECSSystemCamera::GetRegisteredEntity(0);
		CECSComponentBearings *bearingsComponent = CECSComponentBearings::GetByEntityId(cameraEntity);
		Assert_(bearingsComponent != NULL);
		pos = bearingsComponent->m_position;
		fwd = bearingsComponent->m_rotationAxisZ;
		up = bearingsComponent->m_rotationAxisY;
		left = bearingsComponent->m_rotationAxisX;
		sector = bearingsComponent->m_sector;

		return true;
	}
};