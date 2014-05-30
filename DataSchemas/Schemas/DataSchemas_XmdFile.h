/*==================================================================================================

	DataSchemas_XmdFile.h

	This defines the schemas used to load .xmd model files

==================================================================================================*/

SchemaBegin(vert, "Data for a single vertex")
	Field_Schema(Vec3, pos, "0,0,0", "The vertex position")
	Field_Schema(Vec3, normal, "0,0,0", "The normal of the face at this vertex")
	Field_Schema(Vec3, tangent, "0,0,0", "The tangent of the face at this vertex")
	Field_Schema(Vec3, bitangent, "0,0,0", "The bitangent of the face at this vertex")
	Field_Schema(Vec2, uv, "0,0", "The U,V texture coordinates of the face at this vertex")
SchemaEnd

SchemaBegin(face, "A face in an object is made up of an array of 3 vertices")
	Field_Schema_Array(vert, vert, "An array of vertices in the face")
SchemaEnd

SchemaBegin(object, "An object in a model is made up of a mesh of faces")
	Field_Schema_Array(Material, material, "The materials of the object")
	Field_Schema_Array(face, face, "An array of faces in the object")
	Field(bool, CastShadows, true, "Whether or not this object casts a shadow")
SchemaEnd

SchemaBegin(XMDFILE, "Custom model file format")
	Field_Schema_Array(object, object, "An array of objects in the model")
SchemaEnd