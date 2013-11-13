/*==================================================================================================

DataSchemasStructs.h

This expands the schemas defined in DataSchemas.h into structs

==================================================================================================*/
#pragma once

#include <string>
#include <vector>

// Define the structs
#define SchemaBegin(name) struct SData_##name { \
	SData_##name () { SetDefault(); }\
	void SetDefault ();
#define SchemaEnd };
#define Field(type, name, default)	type m_##name;
#define Field_Schema(type, name) SData_##type m_##name;
#define Field_Schema_Array(type, name) std::vector<SData_##type> m_##name;

#include "DataSchemas.h"

#undef SchemaBegin
#undef SchemaEnd
#undef Field
#undef Field_Schema
#undef Field_Schema_Array