/*==================================================================================================

	DataSchemas_World.h

	This defines the schemas used by the world (map) files.

==================================================================================================*/

SchemaBegin(Vec4)
	Field(float, x, 0.0f)
	Field(float, y, 0.0f)
	Field(float, z, 0.0f)
	Field(float, w, 0.0f)
SchemaEnd

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
	Field_Schema(Vec3, ConeDirection, "1,0,0")
	Field(float, ConeAngle, 360.0f)
	Field(float, ConeAttenuationAngle, 360.0f)
	Field(float, ConeFalloffFactor, 0.0f)
	Field(float, AttenuationConstant, 1.0f)
	Field(float, AttenuationDistance, 0.0f)
	Field(float, AttenuationDistanceSquared, 0.0f)
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

SchemaBegin(Portal)
	Field(std::string, id, "")
	Field(std::string, Sector, "")
	Field_Schema(Vec4, XAxis,"1,0,0,0")
	Field_Schema(Vec4, YAxis,"0,1,0,0")
	Field_Schema(Vec4, ZAxis,"0,0,1,0")
	Field_Schema(Vec4, WAxis,"0,0,0,1")
SchemaEnd

SchemaBegin(Box)
	Field_Schema(Vec3, Position, "0,0,0")
	Field_Schema(Vec3, Scale, "1,1,1")
	Field(std::string, Material, "")
	Field_Schema(Vec2, TextureScale, "1,1")
	Field(bool, CastShadows, true)
	Field(std::string, Portal, "")
SchemaEnd

SchemaBegin(Sphere)
	Field_Schema(Vec3, Position, "0,0,0")
	Field(float, Radius, 1.0f)
	Field(std::string, Material, "")
	Field_Schema(Vec2, TextureScale, "1,1")
	Field(bool, CastShadows, true)
	Field(std::string, Portal, "")
SchemaEnd

SchemaBegin(Plane)
	Field_Schema(Vec3, Normal, "0,0,0")
	Field_Schema(Vec3, UAxis, "1,0,0")
	Field_Schema(Vec4, Dimensions, "-10000,-10000,10000,10000")
	Field(float, D, 0.0f)
	Field(std::string, Material, "")
	Field_Schema(Vec2, TextureScale, "1,1")
	Field(bool, CastShadows, true)
	Field(std::string, Portal, "")
SchemaEnd

SchemaBegin(SectorPlane)
	Field_Schema(Vec3, UAxis, "0,0,0")
	Field(std::string, Material, "")
	Field_Schema(Vec2, TextureScale, "1,1")
	Field(std::string, Portal, "")
	Field_Schema(Vec4, PortalWindow, "-10000,-10000,10000,10000")
	Field(std::string, ConnectToSector, "")
	Field(unsigned int, ConnectToSectorPlane, -1)
	Field_Schema(Vec3, ConnectToSectorOffset, "0,0,0")
SchemaEnd

SchemaBegin(Sector)
	Field_Schema(Vec3, AmbientLight, "0,0,0")
	Field_Schema_Array(PointLight, PointLight)
	Field_Schema_Array(Plane, Plane)
	Field_Schema_Array(Box, Box)
	Field_Schema_Array(Sphere, Sphere)
	Field(std::string, id, "")
	Field_Schema(Vec3, Dimensions, "1,1,1")
	Field_Schema_Array(SectorPlane, SectorPlane)
	Field(bool, CastShadows, true)
SchemaEnd

SchemaBegin(Connect)
	Field(std::string, SrcSector, "")
	Field(unsigned int, SrcSectorPlane, -1)
	Field(std::string, DestSector, "")
	Field(unsigned int, DestSectorPlane, -1)
	Field_Schema(Vec3, Offset, "0,0,0")
	Field(bool, BothWays, true)
	Field_Schema(Vec4, PortalWindow, "-10000,-10000,10000,10000")
SchemaEnd

SchemaBegin(World)
	Field(std::string, StartSector, "")
	Field_Schema(Vec3, StartPoint,  "0,0,0")
	Field_Schema(Vec3, StartFacing, "1,0,0")
	Field_Schema_Array(Material, Material)
	Field_Schema_Array(Portal, Portal)
	Field_Schema_Array(Sector, Sector)
	Field_Schema_Array(Connect, Connect)
SchemaEnd