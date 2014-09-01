/*==================================================================================================

ECS.h

The entry point for the ECS system

==================================================================================================*/

#pragma once

#include "DataSchemas/DataSchemasStructs.h"
#include "ECSEnums.h"
#include "Platform/float3.h"

namespace ECS
{
	void CreateEntity (
		unsigned int systems
		#define ComponentBegin(name, hint) , const struct SData_Component##name *component##name
		#include "ECS\ComponentList.h"
	);

	void Update (float elapsedSeconds);

	// Input interface
	void OnMouseMove (float relX, float relY);

	void SetInputState(
	#define INPUT_TOGGLE(name, resetOnKeyUp) bool key##name,
	#include "Game/InputToggleList.h"
	void *dummy);

	bool GetCameraTransform (float3 &pos, float3 &fwd, float3 &up, float3 &left, cl_uint &sector);

};