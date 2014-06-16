/*==================================================================================================

ECS.h

The entry point for the ECS system

==================================================================================================*/

#pragma once

#include "DataSchemas/DataSchemasStructs.h"
#include "ECSEnums.h"

namespace ECS
{
	void CreateEntity (
		unsigned int systems
		#define ComponentBegin(name, hint) , const SData_Component##name *component##name
		#include "ECS\ComponentList.h"
	);

	void Update (float elapsedSeconds);

};