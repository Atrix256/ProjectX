/*==================================================================================================

DataSchemasStructs.cpp

This expands the schemas defined in DataSchemas.h into structs

==================================================================================================*/
#include "DataSchemasStructs.h"

// Define the SetDefault() function
#define SchemaBegin(name) void SData_##name::SetDefault () {
#define SchemaEnd }
#define Field(type, name, default)	m_##name = default;
#define Field_Schema(type, name, default) m_##name.SetDefault();
#define Field_Schema_Array(type, name) m_##name.clear();

#include "DataSchemas.h"

#undef SchemaBegin
#undef SchemaEnd
#undef Field
#undef Field_Schema
#undef Field_Schema_Array

// make the static schema name field
#define SchemaBegin(name) const char *SData_##name::s_schemaName = "SData_" #name;
#define SchemaEnd
#define Field(type, name, default)
#define Field_Schema(type, name, default)
#define Field_Schema_Array(type, name)

#include "DataSchemas.h"

#undef SchemaBegin
#undef SchemaEnd
#undef Field
#undef Field_Schema
#undef Field_Schema_Array
