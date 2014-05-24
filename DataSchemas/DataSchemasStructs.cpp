/*==================================================================================================

DataSchemasStructs.cpp

This expands the schemas defined in DataSchemas.h into structs

==================================================================================================*/
#include "DataSchemasStructs.h"

// Define the SetDefault() function
#define SchemaBegin(name, hint) void SData_##name::SetDefault () {
#define SchemaEnd }
#define Field(type, name, default, hint)	m_##name = default;
#define Field_Schema(type, name, default, hint) m_##name.SetDefault();
#define Field_Schema_Array(type, name, hint) m_##name.clear();
#define Field_Value_Array(type, hint) m_ValueArray.clear();

#include "DataSchemas.h"

#undef SchemaBegin
#undef SchemaEnd
#undef Field
#undef Field_Schema
#undef Field_Schema_Array
#undef Field_Value_Array

// make the static schema name field
#define SchemaBegin(name, hint) const char *SData_##name::s_schemaName = "SData_" #name;
#define SchemaEnd
#define Field(type, name, default, hint)
#define Field_Schema(type, name, default, hint)
#define Field_Schema_Array(type, name, hint)
#define Field_Value_Array(type, hint)

#include "DataSchemas.h"

#undef SchemaBegin
#undef SchemaEnd
#undef Field
#undef Field_Schema
#undef Field_Schema_Array
#undef Field_Value_Array
