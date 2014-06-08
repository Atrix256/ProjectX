/*==================================================================================================

	DataSchemas_World.h

	This defines the schemas used by the world (map) files.

==================================================================================================*/

SchemaBegin(Vec4, "Stores a 4d floating point vector")
	Field(float, x, 0.0f, "x component")
	Field(float, y, 0.0f, "y component")
	Field(float, z, 0.0f, "z component")
	Field(float, w, 0.0f, "w component")
SchemaEnd

SchemaBegin(Vec3, "Stores a 3d floaing point vector")
	Field(float, x, 0.0f, "x component")
	Field(float, y, 0.0f, "y component")
	Field(float, z, 0.0f, "z component")
SchemaEnd

SchemaBegin(Vec2, "Stores a 2d floaing point vector")
	Field(float, x, 0.0f, "x component")
	Field(float, y, 0.0f, "y component")
SchemaEnd

SchemaBegin(PointLight, "Information about a point light")
	Field_Schema(Vec3, Position, "0,0,0", "Light position")
	Field_Schema(Vec3, Color, "1,1,1", "Color of the light")
	Field_Schema(Vec3, ConeDirection, "1,0,0", "Light direction")
	Field(float, ConeAngle, 360.0f, "Angle of the light cone, in degrees. Includes the attenuation angle.")
	Field(float, ConeAttenuationAngle, 360.0f, "Angle of the attenuation zone of the cone light, in degrees.  Must be less than or equal to ConeAngle.")
	Field(float, ConeFalloffFactor, 0.0f, "Exponential power used to determine falloff factor within the attenuation angle.")
	Field(float, AttenuationConstant, 1.0f, "Constant attenuation of the light")
	Field(float, AttenuationDistance, 0.0f, "Linear attenuation of the light over distance")
	Field(float, AttenuationDistanceSquared, 0.0f, "Quadratic attenuation of the light over distance")
SchemaEnd

SchemaBegin(Material, "Information about a surface material")
	Field(std::string, id, "", "The id (unique name) of the material")
	Field_Schema(Vec3, DiffuseColor,"1,1,1", "The diffuse color of the material. Color multiplier for the diffuse texture as well, if one is applied.")
	Field(std::string, DiffuseTexture, "", "The name of the image file to use for the diffuse color of the material.  Leave blank for no texture")
	Field(std::string, NormalTexture, "", "The name of the image file to use to apply normal mapping to the surface of the object")
	Field_Schema(Vec3, SpecularColor, "1,1,1", "The color of the specular highlight of the surface")
	Field(float, SpecularPower, 1.0f, "Higher values make for a smaller, sharper specular highlight.  Lower values make for a larger, duller specular highlight")
	Field_Schema(Vec3, EmissiveColor, "0,0,0", "The emissive color of the material aka the color that shows up when there is no light on the object. Color multiplier for the emissive texture as well, if one is applied.")
	Field(std::string, EmissiveTexture, "", "the name of the image file to sue for the emissive color of the material.  Leave blank for no texture")
	Field_Schema(Vec3, ReflectionColor, "0,0,0", "How reflective the surface should be.  A value of 1 means it will reflect all light, a value of 0 means it will reflect none.  A value of 0.5 means it will reflect the light at half intensity. A material may be EITHER reflective OR refractive (transparent) but not both. If both are specified, the material will be reflective ONLY.")
	Field_Schema(Vec3, RefractionColor, "0,0,0", "How refractive (transparent) the surface should be.  A value of 1 means it will let through all light, a value of 0 means it will let none through.  A value of 0.5 means it will let through all light at half intensity.  Refraction will not work unless there is no reflection since an object may be EITHER reflective OR refractive but not both.")
	Field(float, RefractionIndex, 1.0f, "The index of refraction of the material, aka how much light is bent when it enters and exits the material.  a value of 1 means the light will not be bent at all. Refraction is not active unless refraction amount is also greater than 0 and the object is not reflective.")
	Field_Schema(Vec3, Absorbance, "0,0,0", "Absorbance (A) per centimeter for each color channel (red, green blue). Transmission = 10^(A*cm traveled).")
SchemaEnd

SchemaBegin(Portal, "Information about a portal, which allows you to see, or travel, between sectors")
	Field(std::string, id, "", "the id (unique name) of the portal")
	Field(std::string, Sector, "", "the sector that the portal leads to")
	Field_Schema(Vec4, XAxis,"1,0,0,0", "the X axis of the transform matrix applied to any objects going through the portal, to transform from the old sector's space to the new sector's space.")
	Field_Schema(Vec4, YAxis,"0,1,0,0", "the Y axis of the transform matrix applied to any objects going through the portal, to transform from the old sector's space to the new sector's space.")
	Field_Schema(Vec4, ZAxis,"0,0,1,0", "the Z axis of the transform matrix applied to any objects going through the portal, to transform from the old sector's space to the new sector's space.")
	Field_Schema(Vec4, WAxis,"0,0,0,1", "the W axis of the transform matrix applied to any objects going through the portal, to transform from the old sector's space to the new sector's space."*)
	Field(bool, SetPosition, false, "If true, when an object goes through the portal, it will have the position set to the specified position.  Useful for skybox sectors.")
	Field_Schema(Vec3, Position, "0,0,0", "The position to set an object to if it goes through the portal.  SetPosition must be true for this to happen.")
SchemaEnd

SchemaBegin(Sphere, "Information about a sphere object")
	Field_Schema(Vec3, Position, "0,0,0", "The position of the center point of the sphere")
	Field(float, Radius, 1.0f, "The radius of the sphere")
	Field(std::string, Material, "", "The id (unique name) of the material to use")
	Field_Schema(Vec2, TextureScale, "1,1", "The 2d scaling factor to apply to the u and v axis of the texture")
	Field_Schema(Vec2, TextureOffset, "0,0", "The 2d offset to apply to the u and v axis of the texture")
	Field(bool, CastShadows, true, "Whether or not this object casts a shadow")
	Field(std::string, Portal, "", "The id (unique name) of the portal associated with this object")
SchemaEnd

SchemaBegin(Triangle, "Information about a triangle.  Points are assumed to be in clockwise order.")
	Field_Schema(Vec3, A, "0,0,0", "The first point of the triangle")
	Field_Schema(Vec3, B, "0,0,0", "The second point of the triangle")
	Field_Schema(Vec3, C, "0,0,0", "The third point of the triangle")
	Field_Schema(Vec2, TextureA, "0,0", "The 2d texture coordinate of triangle point A")
	Field_Schema(Vec2, TextureB, "0,0", "The 2d texture coordinate of triangle point B")
	Field_Schema(Vec2, TextureC, "0,0", "The 2d texture coordinate of triangle point C")
	Field(std::string, Material, "", "The id (unique name) of the material to use")
	Field(bool, CastShadows, true, "Whether or not this triangle casts a shadow")
	Field(std::string, Portal, "", "The id (unique name) of the portal associated with this triangle")
SchemaEnd

SchemaBegin(SectorPlane, "Information about a sector wall (there are 6 sector walls for each sector)")
	Field_Schema(Vec3, UAxis, "0,0,0", "The U axis of the sector wall. Used by textures.  The V axis is derived.")
	Field(std::string, Material, "", "The id (unique name) of the material to use")
	Field_Schema(Vec2, TextureScale, "1,1", "The 2d scaling factor to apply to the u and v axis of the texture")
	Field_Schema(Vec2, TextureOffset, "0,0", "The 2d offset to apply to the u and v axis of the texture")
	Field(std::string, Portal, "", "The id (unique name) of the portal associated with this triangle")
	Field_Schema(Vec4, PortalWindow, "-10000,-10000,10000,10000", "The U,V space min/max x/y pair that defines the edges of the portal.  Useful for cutting a hole in a wall to see into a different sector.")
	Field(std::string, ConnectToSector, "", "The id (unique name) of the sector to automatically connect to.  Portals and portal windows will be created as needed to make this happen.")
	Field(unsigned int, ConnectToSectorPlane, -1, "The index of the sector wall to connect to when specifying a ConnectToSector. The walls for the indices are as follows: +x, -x, +y, -y, +z, -z.")
	Field_Schema(Vec2, ConnectToSectorOffset, "0,0", "a U,V space X,Y offset to add to the destination connection point." )
	Field(bool, ConnectToSetPosition, false, "If true, will set the 'set position' flag to true for the generated portal to support the ConnectTo stuff.  Useful for skybox sectors.")
	Field_Schema(Vec3, ConnectToPosition, "0,0,0", "a U,V space X,Y offset to add to the source connection point.")
SchemaEnd

SchemaBegin(ModelInstance, "Information about a mesh model")
	Field(std::string, id, "", "the id (unique name) of the model instance")
	Field(std::string, ModelId, "", "the id (unique name) of the model you want to create an instance of")
	Field(std::string, MaterialOverride, "", "The id (unique name) of the material to override any materials on the model with.")
	Field(std::string, Portal, "", "The id (unique name) of the portal associated with this triangle")
	Field_Schema(Vec3, Position, "0,0,0", "The position of the origin of the model")
	Field_Schema(Vec3, Rotation, "0,0,0", "Rotation around the X,Y,Z axis, in degrees. It applies X axis rotation, then Y axis, then Z axis.")
	Field(float, Scale, 1.0f, "The scale of the model")
SchemaEnd

SchemaBegin(Sector, "Information about a sector.  A sector is a single room in the game world, represented by an axis aligned box which can connect to other sectors.")
	Field_Schema(Vec3, AmbientLight, "0,0,0", "The ambient light for the sector")
	Field_Schema_Array(PointLight, PointLight, "The point lights in the sector")
	Field_Schema_Array(Sphere, Sphere, "The spheres in the sector")
	Field_Schema_Array(ModelInstance, ModelInstance, "The models in the sector")
	Field(std::string, id, "", "The id (unique name) of the sector")
	Field_Schema(Vec3, Dimensions, "1,1,1", "The width, height and depth of the sector's AABB")
	Field_Schema_Array(SectorPlane, SectorPlane, "Information about each wall of the sector")
	Field(bool, FogConstantDensity, false, "If true, FogDensityFactor is used as the constant fog density in the fogged half space.  Otherwise, the density increases by FogDensityFactor as you get deeper into the fogged half space.")
	Field_Schema(Vec3, FogColor, "1,1,1", "The color of fog in the sector.")
	Field(float, FogDensityFactor, 0.0f, "This is how much fog density is added per unit (meter) inside of the fog plane.  Fog density is how much fog applied per world unit (meter) a ray travels.  1.0 = full fog at 1 unit.  0.25 = full fog at 4 units.")
	Field(float, FogFactorMax, 0.8f, "The maximum amount something is allowed to be fogged.  1.0 means it can be completely fogged, 0.25 means it can only be 25% fogged max")
	Field_Schema(Vec4, FogPlane, "0,1,0,0", "The equation of the plane where fog begins.  The first 3 values is the normal of the plane, and the 4th plane is 'D'")
SchemaEnd

/*
SchemaBegin(Connect)
	Field(std::string, SrcSector, "")
	Field(unsigned int, SrcSectorPlane, -1)
	Field(std::string, DestSector, "")
	Field(unsigned int, DestSectorPlane, -1)
	Field_Schema(Vec2, Offset, "0,0")
	Field(bool, BothWays, true)
	Field_Schema(Vec4, PortalWindow, "-10000,-10000,10000,10000")
SchemaEnd
*/

SchemaBegin(Model, "A model to load")
	Field(std::string, id, "", "The id (unique name) of the model")
	Field(std::string, FileName, "", "the filename of the .xmd model file to load")
SchemaEnd

SchemaBegin(World, "The definition of the world")
	Field(std::string, StartSector, "", "The id of sector the player starts in")
	Field_Schema(Vec3, StartPoint,  "0,0,0", "The location the player starts at within the StartSector")
	Field_Schema(Vec3, StartFacing, "1,0,0", "The vector describing the initial rotation of the camera")
	Field_Schema_Array(Model, Model, "The models that can be used in this level")
	Field_Schema_Array(Material, Material, "Definitions for the materials used in the world")
	Field_Schema_Array(Portal, Portal, "Definitions for the portals used in the world")
	Field_Schema_Array(Sector, Sector, "Definitions for the sectors used in the world")
	Field(bool, AutoAdjustBrightness, false, "Whether or not to automatically adjust brightness in this world.  Work in progress.")
	//Field_Schema_Array(Connect, Connect)
SchemaEnd