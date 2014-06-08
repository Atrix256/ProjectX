/*==================================================================================================

DataSchemasStructs.h

This expands the schemas defined in DataSchemas.h into structs

==================================================================================================*/
#pragma once

#include <string>
#include <vector>

namespace SData
{
	template <typename T> 
	inline unsigned int GetEntryById(const std::vector<T>& data, const std::string &id, unsigned int defaultValue, decltype(T::m_id) *p = NULL)
	{
		return GetEntryById(data, id.c_str(), defaultValue);
	}

	template <typename T> 
	inline unsigned int GetEntryById(const std::vector<T>& data, const char *id, unsigned int defaultValue, decltype(T::m_id) *p = NULL)
	{
		if (id && id[0])
		{
			for (unsigned int index = 0, count = data.size(); index < count; ++index)
			{
				if (!strcmp(id, data[index].m_id.c_str()))
					return index;
			}
		}
		return defaultValue;
	}
};

// Define the structs
#define SchemaBegin(name, hint) struct SData_##name { \
	SData_##name () { SetDefault(); }\
	void SetDefault (); \
	static const char *s_schemaName;
#define SchemaEnd };
#define Field(type, name, default, hint)	type m_##name;
#define Field_Schema(type, name, default, hint) SData_##type m_##name;
#define Field_Schema_Array(type, name, hint) \
	std::vector<SData_##type> m_##name;
#define Field_Value_Array(type, hint) \
	std::vector<type> m_ValueArray;

#include "DataSchemas.h"

#undef SchemaBegin
#undef SchemaEnd
#undef Field
#undef Field_Schema
#undef Field_Schema_Array
#undef Field_Value_Array
