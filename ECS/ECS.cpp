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
	static unsigned int s_nextEntityId = 1;
	static bool s_doingUpdate = false;
	static const CSharedArray<SSector> *s_worldsectors = NULL;

	//--------------------------------------------------------------------------------------------------
	void CreateEntity (
		unsigned int systems
		#define ComponentBegin(name, hint) , const struct SData_Component##name *componentData##name
		#include "ECS\ComponentList.h"
	)
	{
		// reserve an entity id
		unsigned int entityId = s_nextEntityId++;

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
		s_doingUpdate = true;

		// update each system - this can be threaded in the future
		#define SystemBegin(name, single, hint) CECSSystem##name::UpdateSystem(elapsedSeconds);
		#include "SystemList.h"

		s_doingUpdate = false;
	}

	//--------------------------------------------------------------------------------------------------
	void OnMouseMove (float relX, float relY)
	{
		// We access components directly without regard for threading matters, so we better not do this
		// during an update call!
		Assert_(s_doingUpdate == false);

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
		Assert_(s_doingUpdate == false);

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

		if (CECSSystemFPSCamera::GetRegisteredEntityCount() == 0)
			return false;

		unsigned int cameraEntity = CECSSystemFPSCamera::GetRegisteredEntity(0);
		CECSComponentBearings &bearingsComponent = CECSComponentBearings::MustGetByEntityId(cameraEntity);
		pos = bearingsComponent.m_position;
		sector = bearingsComponent.m_sector;

		CECSComponentCamera &cameraComponent = CECSComponentCamera::MustGetByEntityId(cameraEntity);

		float3 xAxis, yAxis, zAxis;
		Camera_GetBasisVectors(cameraComponent, xAxis, yAxis, zAxis);

		left = xAxis;
		up = yAxis;
		fwd = zAxis;

		return true;
	}

	//--------------------------------------------------------------------------------------------------
	void SetWorldData (const CSharedArray<SSector> &sectors)
	{
		s_worldsectors = &sectors;
	}

	//--------------------------------------------------------------------------------------------------
	const CSharedArray<SSector> &GetWorldData ()
	{
		Assert_(s_worldsectors != NULL);
		return *s_worldsectors;
	}
};