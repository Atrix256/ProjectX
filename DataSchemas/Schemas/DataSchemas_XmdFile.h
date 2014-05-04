/*==================================================================================================

	DataSchemas_XmdFile.h

	This defines the schemas used to load .xmd model files

==================================================================================================*/

SchemaBegin(vert)
	Field_Schema(Vec3, pos, "0,0,0")
	Field_Schema(Vec3, normal, "0,0,0")
	Field_Schema(Vec3, tangent, "0,0,0")
	Field_Schema(Vec3, bitangent, "0,0,0")
	Field_Schema(Vec2, uv, "0,0")
SchemaEnd

SchemaBegin(face)
	Field_Schema_Array(vert, vert)
SchemaEnd

SchemaBegin(object)
	Field_Schema_Array(face, face)
SchemaEnd

SchemaBegin(XMDFILE)
	Field_Schema_Array(object, object)
SchemaEnd