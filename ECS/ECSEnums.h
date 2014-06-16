/*==================================================================================================

ECS.h

Enums and constants for the ECS system

==================================================================================================*/

#pragma once

namespace ECS
{
	enum ESystems
	{
		e_systemUnknown = -1,

		#define SystemBegin(name, single, hint) e_system##name,
		#include "SystemList.h"

		e_systemCount
	};

	enum ESystemFlags
	{
		e_systemFlagNone = 0,

		#define SystemBegin(name, single, hint) e_systemFlag##name = (1 << e_system##name),
		#include "SystemList.h"

		e_systemFlagAll = -1,
	};
};