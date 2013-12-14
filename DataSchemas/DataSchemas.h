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

Field_Schema_Array(type, name) - define a field in the schema that is an array of schema types

==================================================================================================*/

SchemaBegin(Vec3)
	Field(float, x, 0.0f)
	Field(float, y, 0.0f)
	Field(float, z, 0.0f)
SchemaEnd

SchemaBegin(Vec2)
	Field(float, x, 0.0f)
	Field(float, y, 0.0f)
SchemaEnd

SchemaBegin(PointLight)
	Field_Schema(Vec3, Position, "0,0,0")
	Field_Schema(Vec3, Color, "1,1,1")
SchemaEnd

SchemaBegin(Material)
	Field(std::string, id, "")
	Field_Schema(Vec3, DiffuseColor,"1,1,1")
	Field(std::string, DiffuseTexture, "")
	Field(std::string, NormalTexture, "")
	Field_Schema(Vec3, SpecularColor, "1,1,1")
	Field(float, SpecularPower, 1.0f)
	Field_Schema(Vec3, EmissiveColor, "0,0,0")
	Field(std::string, EmissiveTexture, "")
	Field(float, ReflectionAmount, 0.0f)
	Field(float, RefractionIndex, 1.0f)
	Field(float, RefractionAmount, 0.0f)
SchemaEnd

SchemaBegin(Box)
	Field_Schema(Vec3, Position, "0,0,0")
	Field_Schema(Vec3, Scale, "1,1,1")
	Field(std::string, Material, "")
	Field_Schema(Vec2, TextureScale, "1,1")
	Field(bool, CastShadows, true)
SchemaEnd

SchemaBegin(Sphere)
	Field_Schema(Vec3, Position, "0,0,0")
	Field(float, Radius, 1.0f)
	Field(std::string, Material, "")
	Field_Schema(Vec2, TextureScale, "1,1")
	Field(bool, CastShadows, true)
SchemaEnd

SchemaBegin(Plane)
	Field_Schema(Vec3, Normal, "0,0,0")
	Field_Schema(Vec3, UAxis, "0,0,0")
	Field(float, D, 0.0f)
	Field(std::string, Material, "")
	Field_Schema(Vec2, TextureScale, "1,1")
	Field(bool, CastShadows, true)
SchemaEnd

SchemaBegin(World)
	Field_Schema(Vec3, StartPoint, "0,0,0")
	Field(float, StartFacing, 0.0f)
	Field_Schema(Vec3, AmbientLight, "0,0,0")
	Field_Schema_Array(PointLight, PointLight)
	Field_Schema_Array(Material, Material)
	Field_Schema_Array(Box, Box)
	Field_Schema_Array(Sphere, Sphere)
	Field_Schema_Array(Plane, Plane)
SchemaEnd