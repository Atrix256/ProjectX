/*==================================================================================================

CWorld.cpp

This class holds all information about the world

==================================================================================================*/

#include "Platform\CDirectx.h"

#include "CWorld.h"
#include "CGame.h"

#include "DataSchemas/DataSchemasXML.h"

//-----------------------------------------------------------------------------
void Copy(cl_float2 &lhs, const SData_Vec2 &rhs)
{
	lhs.s[0] = rhs.m_x;
	lhs.s[1] = rhs.m_y;
}

//-----------------------------------------------------------------------------
void Copy(float3 &lhs, const SData_Vec3 &rhs)
{
	lhs[0] = rhs.m_x;
	lhs[1] = rhs.m_y;
	lhs[2] = rhs.m_z;
}

//-----------------------------------------------------------------------------
void Copy(cl_float4 &lhs, const SData_Vec3 &rhs, const float w)
{
	lhs.s[0] = rhs.m_x;
	lhs.s[1] = rhs.m_y;
	lhs.s[2] = rhs.m_z;
	lhs.s[3] = w;
}

//-----------------------------------------------------------------------------
void Copy(cl_float4 &lhs, const SData_Vec4 &rhs)
{
	lhs.s[0] = rhs.m_x;
	lhs.s[1] = rhs.m_y;
	lhs.s[2] = rhs.m_z;
	lhs.s[3] = rhs.m_w;
}

//-----------------------------------------------------------------------------
void Normalize(float3 &vec)
{
	float mag = sqrtf(
		vec[0] * vec[0] + 
		vec[1] * vec[1] + 
		vec[2] * vec[2]);

	if (mag != 0)
	{
		vec[0] /= mag;
		vec[1] /= mag;
		vec[2] /= mag;
	}
}

//-----------------------------------------------------------------------------
void Normalize(cl_float4 &vec)
{
	float mag = sqrtf(
		vec.s[0] * vec.s[0] + 
		vec.s[1] * vec.s[1] + 
		vec.s[2] * vec.s[2]);

	if (mag != 0)
	{
		vec.s[0] /= mag;
		vec.s[1] /= mag;
		vec.s[2] /= mag;
	}
}

//-----------------------------------------------------------------------------
bool CWorld::Load(const char *worldFileName)
{
	SData_World worldData;
	if (!DataSchemasXML::Load(worldData, worldFileName, "World"))
		worldData.SetDefault();

	// Starting Position and facing
	CGame::SetPlayerPos(worldData.m_StartPoint.m_x, worldData.m_StartPoint.m_y, worldData.m_StartPoint.m_z);
	CGame::SetPlayerFacing(worldData.m_StartFacing * 3.1415f / 180.0f);

	// point lights
	m_pointLights.Resize(worldData.m_PointLight.size());
	for (unsigned int index = 0, count = worldData.m_PointLight.size(); index < count; ++index)
	{
		Copy(m_pointLights[index].m_color, worldData.m_PointLight[index].m_Color);
		Copy(m_pointLights[index].m_position, worldData.m_PointLight[index].m_Position);
	}

	// materials
	m_materials.Resize(worldData.m_Material.size());
	for (unsigned int index = 0, count = worldData.m_Material.size(); index < count; ++index)
	{
		Copy(m_materials[index].m_diffuseColor, worldData.m_Material[index].m_DiffuseColor);
		Copy(m_materials[index].m_specularColorAndPower, worldData.m_Material[index].m_SpecularColor, worldData.m_Material[index].m_SpecularPower);
		Copy(m_materials[index].m_emissiveColor, worldData.m_Material[index].m_EmissiveColor);
		m_materials[index].m_reflectionAmount = worldData.m_Material[index].m_ReflectionAmount;
		m_materials[index].m_refractionAmount = worldData.m_Material[index].m_RefractionAmount;
		m_materials[index].m_refractionIndex = worldData.m_Material[index].m_RefractionIndex;
		
		m_materials[index].m_diffuseTextureIndex = 0.0f;
		if (worldData.m_Material[index].m_DiffuseTexture.length() > 0)
			m_materials[index].m_diffuseTextureIndex = (float)CDirectX::TextureManager().GetOrLoad(worldData.m_Material[index].m_DiffuseTexture.c_str());

		m_materials[index].m_normalTextureIndex = 0.0f;
		if (worldData.m_Material[index].m_NormalTexture.length() > 0)
			m_materials[index].m_normalTextureIndex = (float)CDirectX::TextureManager().GetOrLoad(worldData.m_Material[index].m_NormalTexture.c_str());

		m_materials[index].m_emissiveTextureIndex = 0.0f;
		if (worldData.m_Material[index].m_EmissiveTexture.length() > 0)
			m_materials[index].m_emissiveTextureIndex = (float)CDirectX::TextureManager().GetOrLoad(worldData.m_Material[index].m_EmissiveTexture.c_str());

	}

	unsigned int nextObjectId = 1;

	// boxes
	m_boxes.Resize(worldData.m_Box.size());
	for (unsigned int index = 0, count = worldData.m_Box.size(); index < count; ++index)
	{
		m_boxes[index].m_objectId = nextObjectId++;
		Copy(m_boxes[index].m_position, worldData.m_Box[index].m_Position);
		Copy(m_boxes[index].m_scale, worldData.m_Box[index].m_Scale);
		m_boxes[index].m_castsShadows = worldData.m_Box[index].m_CastShadows;
		Copy(m_boxes[index].m_textureScale, worldData.m_Box[index].m_TextureScale);

		// set the material index
		m_boxes[index].m_materialIndex = SData::GetEntryById(worldData.m_Material, worldData.m_Box[index].m_Material);
	}

	// spheres
	m_spheres.Resize(worldData.m_Sphere.size());
	for (unsigned int index = 0, count = worldData.m_Sphere.size(); index < count; ++index)
	{
		m_spheres[index].m_objectId = nextObjectId++;
		Copy(m_spheres[index].m_positionAndRadius, worldData.m_Sphere[index].m_Position, worldData.m_Sphere[index].m_Radius);
		m_spheres[index].m_castsShadows = worldData.m_Sphere[index].m_CastShadows;
		Copy(m_spheres[index].m_textureScale, worldData.m_Sphere[index].m_TextureScale);

		// set the material index
		m_spheres[index].m_materialIndex = SData::GetEntryById(worldData.m_Material, worldData.m_Sphere[index].m_Material);
	}

	// planes
	m_planes.Resize(worldData.m_Plane.size());
	for (unsigned int index = 0, count = worldData.m_Plane.size(); index < count; ++index)
	{
		m_planes[index].m_objectId = nextObjectId++;
		Copy(m_planes[index].m_equation, worldData.m_Plane[index].m_Normal, worldData.m_Plane[index].m_D);
		Copy(m_planes[index].m_UAxis, worldData.m_Plane[index].m_UAxis);
		m_planes[index].m_castsShadows = worldData.m_Plane[index].m_CastShadows;
		Copy(m_planes[index].m_textureScale, worldData.m_Plane[index].m_TextureScale);
		m_planes[index].m_dims.s[0] = worldData.m_Plane[index].m_Dimensions.m_x;
		m_planes[index].m_dims.s[1] = worldData.m_Plane[index].m_Dimensions.m_y;
		m_planes[index].m_dims.s[2] = worldData.m_Plane[index].m_Dimensions.m_z;
		m_planes[index].m_dims.s[3] = worldData.m_Plane[index].m_Dimensions.m_w;

		// make sure the normal is normalized in the equation and normalize the U axis
		Normalize(m_planes[index].m_equation);
		Normalize(m_planes[index].m_UAxis);

		// set the material index
		m_planes[index].m_materialIndex = SData::GetEntryById(worldData.m_Material, worldData.m_Plane[index].m_Material);
	}

	// sectors
	m_sectors.Resize(worldData.m_Sector.size());
	for (unsigned int index = 0, count = worldData.m_Sector.size(); index < count; ++index)
	{
		Assert_(worldData.m_Sector[index].m_Plane.size() == 6);

		Copy(m_sectors[index].m_halfDims, worldData.m_Sector[index].m_Dimensions);
		m_sectors[index].m_halfDims /= 2.0f;

		m_sectors[index].m_castsShadows = worldData.m_Sector[index].m_CastShadows;

		for (unsigned int planeIndex = 0; planeIndex < SSECTOR_NUMPLANES; ++planeIndex)
		{
			m_sectors[index].m_planes[planeIndex].m_objectId = nextObjectId++;

			Copy(m_sectors[index].m_planes[planeIndex].m_UAxis, worldData.m_Sector[index].m_Plane[planeIndex].m_UAxis);
			Copy(m_sectors[index].m_planes[planeIndex].m_textureScale, worldData.m_Sector[index].m_Plane[planeIndex].m_TextureScale);

			// set the next sector
			m_sectors[index].m_planes[planeIndex].m_portalNextSector = SData::GetEntryById(worldData.m_Sector, worldData.m_Sector[index].m_Plane[planeIndex].m_PortalNextSector);
			Copy(m_sectors[index].m_planes[planeIndex].m_portalWindow, worldData.m_Sector[index].m_Plane[planeIndex].m_PortalWindow);

			// make sure the Uaxis is normalized
			Normalize(m_sectors[index].m_planes[planeIndex].m_UAxis);

			// set the material index
			m_sectors[index].m_planes[planeIndex].m_materialIndex = SData::GetEntryById(worldData.m_Material, worldData.m_Sector[index].m_Plane[planeIndex].m_Material);
		}
	}

	// calculate our texture indices
	for (unsigned int index = 0, count = worldData.m_Material.size(); index < count; ++index)
	{
		m_materials[index].m_diffuseTextureIndex = (m_materials[index].m_diffuseTextureIndex - 1.0f) / (float)CDirectX::TextureManager().NumTextures();
		m_materials[index].m_diffuseTextureIndex += 0.01f;

		m_materials[index].m_normalTextureIndex = (m_materials[index].m_normalTextureIndex - 1.0f) / (float)CDirectX::TextureManager().NumTextures();
		m_materials[index].m_normalTextureIndex += 0.01f;

		m_materials[index].m_emissiveTextureIndex = (m_materials[index].m_emissiveTextureIndex - 1.0f) / (float)CDirectX::TextureManager().NumTextures();
		m_materials[index].m_emissiveTextureIndex += 0.01f;
	}

	// combine all the textures now that they are all loaded
	CDirectX::TextureManager().FinalizeTextures();

	// set our shared data values
	SWorld &worldShared = SSharedDataRoot::World();
	Copy(worldShared.m_ambientLight, worldData.m_AmbientLight);
	worldShared.m_numLights = m_pointLights.Count();
	worldShared.m_numSpheres = m_spheres.Count();
	worldShared.m_numBoxes = m_boxes.Count();
	worldShared.m_numPlanes = m_planes.Count();
	worldShared.m_numSectors = m_sectors.Count();
	worldShared.m_numMaterials = m_materials.Count();
	return true;
}