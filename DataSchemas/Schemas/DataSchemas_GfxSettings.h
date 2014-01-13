/*==================================================================================================

	DataSchemas_GfxSettings.h

	This defines the schemas used by the graphics settings file.

==================================================================================================*/

SchemaBegin(GfxSettings)
	Field_Schema(Vec2, Resolution, "1000, 1000")
	Field(std::string, DefaultMap, "./data/default.xml")
	Field(unsigned int, TextureSize, 512)
	Field(bool, InterlaceMode, false)
SchemaEnd