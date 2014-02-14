/*==================================================================================================

	DataSchemas_GfxSettings.h

	This defines the schemas used by the graphics settings file.

==================================================================================================*/

SchemaBegin(GfxSettings)
	Field_Schema(Vec2, Resolution, "1000, 1000")
	Field(std::string, DefaultMap, "./data/default.xml")
	Field(unsigned int, TextureSize, 512)
	Field(bool, InterlaceMode, false)
	Field(bool, NormalMapping, true)
	Field(bool, Shadows, true)
	Field(bool, HighQualityLights, true)
	Field(bool, RedBlue3D, false)
	Field(float, RedBlueWidth, -0.4f)
	Field(unsigned int, RayBounces, 10)
	Field(bool, FastestMath, true)
	Field(float, Brightness, 1.0f)
SchemaEnd