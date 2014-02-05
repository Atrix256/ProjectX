/*==================================================================================================

	DataSchemas_DaeFile.h

	This defines the schemas used to load .dae model files

==================================================================================================*/

SchemaBegin(float_array)
	Field(std::string, id, "")
	Field_Value_Array(float)
SchemaEnd

SchemaBegin(source)
	Field(std::string, id, "")
	Field_Schema(float_array, float_array, NULL)
SchemaEnd

SchemaBegin(input)
	Field(std::string, semantic, "")
	Field(std::string, source, "")
	Field(unsigned int, offset, 0)
SchemaEnd

SchemaBegin(vertices)
	Field(std::string, id, "")
	Field_Schema_Array(input, input)
SchemaEnd

SchemaBegin(vcount)
	Field_Value_Array(unsigned int)
SchemaEnd

SchemaBegin(p)
	Field_Value_Array(unsigned int)
SchemaEnd

SchemaBegin(polylist)
	Field_Schema_Array(input, input)
	Field_Schema(vcount, vcount, NULL)
	Field_Schema(p, p, NULL)
SchemaEnd

SchemaBegin(mesh)
	Field_Schema_Array(source, source)
	Field_Schema(vertices, vertices, NULL)
	Field_Schema(polylist, polylist, NULL)
SchemaEnd

SchemaBegin(geometry)
	Field(std::string, id, "")
	Field_Schema(mesh, mesh, NULL)
SchemaEnd

SchemaBegin(library_geometries)
	Field_Schema(geometry, geometry, NULL)
SchemaEnd

SchemaBegin(DAEFILE)
	Field_Schema(library_geometries, library_geometries, NULL)
SchemaEnd