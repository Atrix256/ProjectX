/*==================================================================================================

	DataSchemas_GfxSettings.h

	This defines the schemas used by the graphics settings file.

==================================================================================================*/

SchemaBegin(GfxSettings, "Graphics Settings")
	Field_Schema(Vec2, Resolution, "1000, 1000", "The width and height of the game window")
	Field(bool, FullScreen, false, "Whether or not the game should run in full screen mode")
	Field(std::string, DefaultMap, "./data/default.xml", "The map to load on startup")
	Field(unsigned int, TextureSize, 512, "The width and height resolution of the textures stored in memory.  Make it smaller for less detail but less texture memory used.")
	Field(bool, TextureFilter, true, "If true, will interpolate pixel values to smooth out texture sampling")
	Field(bool, InterlaceMode, false, "If true, will alternate between rendering the top half and the bottom half of the screen every frame.  Boosts performance!")
	Field(bool, NormalMapping, true, "If false, normal mapping will be disabled")
	Field(bool, Shadows, true, "If false, shadows will be disabled.  Big performance boost")
	Field(bool, HighQualityLights, true, "If false, lower quality lighting will be used")
	Field(bool, RedBlue3D, false, "If true, the game will render in red/blue 3d glasses mode.")
	Field(float, RedBlueWidth, -0.4f, "The distance between the left and right eye when rendering in red/blue 3d glasses mode.")
	Field(unsigned int, RayBounces, 10, "The maximum times a ray may bounce in a scene while rendering")
	Field(bool, FastestMath, true, "If true, the fastest (and least precise) math will be used")
	Field(float, Brightness, 1.0f, "Used to adjust brightness")
	Field(bool, ColorAbsorption, true, "If false, color absorption will be off for transparent objects")

	Field(bool, DebugRayBounceCount, false, "If true, will make pixels lighter the more ray bounces were required.  When hitting RayBounces (max) it will add white to the pixel.")
	Field(bool, DebugModelBoundingSphere, false, "If true, will visualize where the bounding spheres of models are - showing which rays tested against which meshes.  It will show rays that only tested upper half resident polygons in green, rays that only tested lower half resident polygons in red, and rays that tested all polygons in white")
	Field(bool, DebugTextureUV, false, "If true, shows the U,V texture coordinates as Red,Green diffuse color instead of doing a texture lookup")
	Field(bool, DebugTriangles, false, "If true, shows triangle geometry")
SchemaEnd