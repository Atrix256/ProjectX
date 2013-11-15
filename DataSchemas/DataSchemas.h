/*==================================================================================================

DataSchemas.h

This defines data structures as macros which can then be expanded into things like data structures,
xml parsing code, network message serialization, and save game serialization.

SchemaBegin(name) - start a schema with the specified name
SchemaEnd() - end the current schema definition

Field(type, name, default) - define a simple (POD) field in the schema.

Field_Schema(type, name) - define a field in the schema that is of a type defined by another schema

Field_Schema_Array(type, name) - define a field in the schema that is an array of schema types

==================================================================================================*/

SchemaBegin(Vec3)
	Field(float, x, 0.0f)
	Field(float, y, 0.0f)
	Field(float, z, 0.0f)
SchemaEnd

SchemaBegin(PointLight)
	Field_Schema(Vec3, Position)
	Field_Schema(Vec3, Color)
SchemaEnd

SchemaBegin(Material)
	Field(std::string, id, "")
	Field_Schema(Vec3, DiffuseColor)
	Field(std::string, DiffuseTexture, "")
	Field_Schema(Vec3, SpecularColor)
	Field(float, SpecularPower, 1.0f)
	Field_Schema(Vec3, EmissiveColor)
	Field(float, ReflectionAmount, 0.0f)
	Field(float, RefractionIndex, 1.0f)
	Field(float, RefractionAmount, 0.0f)
SchemaEnd

SchemaBegin(Box)
	Field_Schema(Vec3, Position)
	Field_Schema(Vec3, Scale)
	Field(std::string, Material, "")
	Field(bool, CastShadows, true)
SchemaEnd

SchemaBegin(Sphere)
	Field_Schema(Vec3, Position)
	Field(float, Radius, 1.0f)
	Field(std::string, Material, "")
	Field(bool, CastShadows, true)
SchemaEnd

SchemaBegin(World)
	Field_Schema(Vec3, StartPoint)
	Field(float, StartFacing, 0.0f)
	Field_Schema(Vec3, AmbientLight)
	Field_Schema_Array(PointLight, PointLight)
	Field_Schema_Array(Material, Material)
	Field_Schema_Array(Box, Box)
	Field_Schema_Array(Sphere, Sphere)
SchemaEnd