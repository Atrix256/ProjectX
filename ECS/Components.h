/*==================================================================================================

Components.h

The code for "Components", which are part of the ECS Entity-Component-Systems model.  Components
hold data for a specific entity and have no logic on them.  They are read and written too by various
systems to do work on entities.

==================================================================================================*/

#pragma once

#include "Platform/float3.h"
#include <vector>

#define ComponentBegin(name, hint) \
class CECSComponent##name \
{ \
public: \
	typedef std::vector<CECSComponent##name *> TList; \
private: \
	CECSComponent##name (unsigned int entityId, const struct SData_Component##name &data); \
	static TList s_components; \
public: \
	static CECSComponent##name *Create (unsigned int entityId, const struct SData_Component##name &data) \
	{ \
		CECSComponent##name *newComponent = new CECSComponent##name(entityId, data); \
		s_components.push_back(newComponent); \
		return newComponent; \
	} \
	static CECSComponent##name *GetByEntityId (unsigned int entityId) \
	{ \
		for (TList::iterator it = s_components.begin(); it != s_components.end(); ++it) \
		{ \
			if ((*it)->m_entityId == entityId) \
				return *it; \
		} \
		return NULL; \
	} \
	static CECSComponent##name &MustGetByEntityId (unsigned int entityId) \
	{ \
		CECSComponent##name *ret = GetByEntityId(entityId); \
		Assert_(ret != NULL); \
		return *ret; \
	} \
	static TList &All () \
	{ \
		return s_components;\
	} \
	unsigned int m_entityId; \

#define ComponentData(type, name, hint) type m_##name;

#define ComponentEnd };

#include "ComponentList.h"

//--------------------------------------------------------------------------------------------------
// Component Helper Functions
// Should probably only have functions that get passed const component references
//--------------------------------------------------------------------------------------------------

void Camera_GetBasisVectors (const CECSComponentCamera &camera, float3 &xAxis, float3 &yAxis, float3 &zAxis);