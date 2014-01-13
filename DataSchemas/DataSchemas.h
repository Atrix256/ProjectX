/*==================================================================================================

DataSchemas.h

This defines data structures as macros which can then be expanded into things like data structures,
xml parsing code, network message serialization, and save game serialization.

SchemaBegin(name) - start a schema with the specified name
SchemaEnd() - end the current schema definition

Field(type, name, default) - define a simple (POD) field in the schema.

Field_Schema(type, name, default) - define a field in the schema that is of a type defined by another
	schema.  If you give a string for default it will try to load it as a string.  If you give a null
	it won't try to load a default.  Note the default only works for xml loaded objects.

Field_Schema_Array(type, name) - define a field in the schema that is an array of schema types.
	Note that if the type has a field named "id", it will enforce that the id is unique on load,
	and the type will be usable with the SData::GetEntryById() function.

==================================================================================================*/

#include "Schemas/DataSchemas_World.h"
#include "Schemas/DataSchemas_GfxSettings.h"
#include "Schemas/DataSchemas_GameData.h"

