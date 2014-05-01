/*==================================================================================================

	DataSchemas_XmdFile.h

	This defines the schemas used to load .xmd model files

==================================================================================================*/

SchemaBegin(face)
	Field_Schema_Array(Vec3, vert)
	Field_Schema_Array(Vec2, uv)
SchemaEnd

SchemaBegin(object)
	Field_Schema_Array(face, face)
SchemaEnd

SchemaBegin(XMDFILE)
	Field_Schema_Array(object, object)
SchemaEnd