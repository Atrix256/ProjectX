/*==================================================================================================

Systems.h

The code for "Systems", which are part of the ECS Entity-Component-Systems model.

==================================================================================================*/

#pragma once

#include <vector>

// define a class for each system
#define SystemBegin(name, single, hint) \
class CECSSystem##name \
{ \
private: \
	CECSSystem##name (); /* don't allow instantiation, systems are static classes*/ \
	static std::vector<unsigned int> s_registeredEntities; \
	static void Update (float elapsedSeconds

#define SYSTEM_READONLY const
#define SYSTEM_READWRITE
#define SystemComponent(type, access, hint) \
	, access class CECSComponent##type &type

#define SystemEnd \
	); \
public: \
	static void Update (float elapsedSeconds); \
	static void Register (unsigned int entityId) {s_registeredEntities.push_back(entityId);} \
};

#include "SystemList.h"