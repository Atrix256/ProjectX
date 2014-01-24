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
void Normalize4D(cl_float4 &vec)
{
	float mag = sqrtf(
		vec.s[0] * vec.s[0] + 
		vec.s[1] * vec.s[1] + 
		vec.s[2] * vec.s[2] + 
		vec.s[3] * vec.s[3]);

	if (mag != 0)
	{
		vec.s[0] /= mag;
		vec.s[1] /= mag;
		vec.s[2] /= mag;
		vec.s[3] /= mag;
	}
}

//-----------------------------------------------------------------------------
void CWorld::LoadSectorPlanes (
	SSector &sector,
	struct SData_Sector &sectorSource,
	std::vector<struct SData_Material> &materials,
	std::vector<struct SData_Portal> &portals
) {
	// load the planes geometry entries
	sector.m_staticPlaneStartIndex = m_planes.Count();
	m_planes.Resize(m_planes.Count() + sectorSource.m_Plane.size());
	for (unsigned int planeIndex = 0, planeCount = sectorSource.m_Plane.size(); planeIndex < planeCount; ++planeIndex)
	{
		SPlane &plane = m_planes[sector.m_staticPlaneStartIndex + planeIndex];
		SData_Plane &planeSource = sectorSource.m_Plane[planeIndex];
		plane.m_objectId = m_nextObjectId++;
		Copy(plane.m_equation, planeSource.m_Normal, planeSource.m_D);
		Copy(plane.m_UAxis, planeSource.m_UAxis);
		plane.m_castsShadows = planeSource.m_CastShadows;
		Copy(plane.m_textureScale, planeSource.m_TextureScale);
		plane.m_dims.s[0] = planeSource.m_Dimensions.m_x;
		plane.m_dims.s[1] = planeSource.m_Dimensions.m_y;
		plane.m_dims.s[2] = planeSource.m_Dimensions.m_z;
		plane.m_dims.s[3] = planeSource.m_Dimensions.m_w;

		// make sure the normal is normalized in the equation and normalize the U axis
		Normalize(plane.m_equation);
		Normalize(plane.m_UAxis);

		// set the material index
		plane.m_materialIndex = SData::GetEntryById(materials, planeSource.m_Material);

		// set the portal index
		plane.m_portalIndex = SData::GetEntryById(portals, planeSource.m_Portal);
	}
	sector.m_staticPlaneStopIndex = m_planes.Count();
}

//-----------------------------------------------------------------------------
void CWorld::LoadSectorBoxes (
	SSector &sector,
	struct SData_Sector &sectorSource,
	std::vector<struct SData_Material> &materials,
	std::vector<struct SData_Portal> &portals
) {
	// load the box geometry entries
	sector.m_staticBoxStartIndex = m_boxes.Count();
	m_boxes.Resize(m_boxes.Count() + sectorSource.m_Box.size());
	for (unsigned int index = 0, count = sectorSource.m_Box.size(); index < count; ++index)
	{
		SAABox &box = m_boxes[sector.m_staticBoxStartIndex + index];
		SData_Box &boxSource = sectorSource.m_Box[index];
		box.m_objectId = m_nextObjectId++;
		Copy(box.m_position, boxSource.m_Position);
		Copy(box.m_scale, boxSource.m_Scale);
		box.m_castsShadows = boxSource.m_CastShadows;
		Copy(box.m_textureScale, boxSource.m_TextureScale);

		// set the material index
		box.m_materialIndex = SData::GetEntryById(materials, boxSource.m_Material);

		// set the portal index
		box.m_portalIndex = SData::GetEntryById(portals, boxSource.m_Portal);
	}
	sector.m_staticBoxStopIndex = m_boxes.Count();
}

//-----------------------------------------------------------------------------
void CWorld::LoadSectorSpheres (
	SSector &sector,
	struct SData_Sector &sectorSource,
	std::vector<struct SData_Material> &materials,
	std::vector<struct SData_Portal> &portals
) {
	// load the sphere geometry entries
	sector.m_staticSphereStartIndex = m_spheres.Count();
	m_spheres.Resize(m_spheres.Count() + sectorSource.m_Sphere.size());
	for (unsigned int index = 0, count = sectorSource.m_Sphere.size(); index < count; ++index)
	{
		SSphere &sphere = m_spheres[sector.m_staticSphereStartIndex + index];
		SData_Sphere &sphereSource = sectorSource.m_Sphere[index];
		sphere.m_objectId = m_nextObjectId++;
		Copy(sphere.m_positionAndRadius, sphereSource.m_Position, sphereSource.m_Radius);
		sphere.m_castsShadows = sphereSource.m_CastShadows;
		Copy(sphere.m_textureScale, sphereSource.m_TextureScale);

		// set the material index
		sphere.m_materialIndex = SData::GetEntryById(materials, sphereSource.m_Material);

		// set the portal index
		sphere.m_portalIndex = SData::GetEntryById(portals, sphereSource.m_Portal);
	}
	sector.m_staticSphereStopIndex = m_spheres.Count();
}

//-----------------------------------------------------------------------------
void CWorld::LoadSectorPointLights (
	SSector &sector,
	struct SData_Sector &sectorSource,
	std::vector<struct SData_Material> &materials,
	std::vector<struct SData_Portal> &portals
) {
	// load the point light entries
	sector.m_staticLightStartIndex = m_pointLights.Count();	
	m_pointLights.Resize(m_pointLights.Count() + sectorSource.m_PointLight.size());
	for (unsigned int index = 0, count = sectorSource.m_PointLight.size(); index < count; ++index)
	{
		SPointLight &light = m_pointLights[sector.m_staticLightStartIndex + index];
		SData_PointLight &lightSource = sectorSource.m_PointLight[index];
		Copy(light.m_color, lightSource.m_Color);
		Copy(light.m_positionAndAttenuationAngle, lightSource.m_Position, -cos((lightSource.m_ConeAngle - lightSource.m_ConeAttenuationAngle) * 3.14f / 180.0f));
		Copy(light.m_coneDirAndAngle, lightSource.m_ConeDirection, -cos(lightSource.m_ConeAngle * 3.14f / 180.0f));
		Normalize(light.m_coneDirAndAngle);
	}
	sector.m_staticLightStopIndex = m_pointLights.Count();
}

//-----------------------------------------------------------------------------
void CWorld::LoadSector (
	SSector &sector,
	struct SData_Sector &sectorSource,
	std::vector<struct SData_Material> &materials,
	std::vector<struct SData_Portal> &portals
) {
	Assert_(sectorSource.m_SectorPlane.size() == 6);

	// load / set the sector data
	Copy(sector.m_ambientLight, sectorSource.m_AmbientLight);
	Copy(sector.m_halfDims, sectorSource.m_Dimensions);
	sector.m_halfDims /= 2.0f;
	sector.m_castsShadows = sectorSource.m_CastShadows;

	// load the sector planes data
	for (unsigned int planeIndex = 0; planeIndex < SSECTOR_NUMPLANES; ++planeIndex)
	{
		SSectorPlane &plane = sector.m_planes[planeIndex];
		SData_SectorPlane &planeSource = sectorSource.m_SectorPlane[planeIndex];
		plane.m_objectId = m_nextObjectId++;

		Copy(plane.m_UAxis, planeSource.m_UAxis);
		Copy(plane.m_textureScale, planeSource.m_TextureScale);

		// copy the portal window
		Copy(plane.m_portalWindow, planeSource.m_PortalWindow);

		// make sure the Uaxis is normalized
		Normalize(plane.m_UAxis);

		// set the material index
		plane.m_materialIndex = SData::GetEntryById(materials, planeSource.m_Material);

		// set the portal index
		plane.m_portalIndex = SData::GetEntryById(portals, planeSource.m_Portal);
	}

	LoadSectorPointLights(sector, sectorSource, materials, portals);
	LoadSectorPlanes(sector, sectorSource, materials, portals);
	LoadSectorBoxes(sector, sectorSource, materials, portals);
	LoadSectorSpheres(sector, sectorSource, materials, portals);
}

//-----------------------------------------------------------------------------
bool CWorld::Load (const char *worldFileName)
{
	SData_World worldData;
	if (!DataSchemasXML::Load(worldData, worldFileName, "World"))
		worldData.SetDefault();

	// Starting Sector, Position and facing
	SSharedDataRoot::Camera().m_sector = SData::GetEntryById(worldData.m_Sector, worldData.m_StartSector);
	CGame::SetPlayerPos(worldData.m_StartPoint.m_x, worldData.m_StartPoint.m_y, worldData.m_StartPoint.m_z);
	CGame::SetPlayerFacing(worldData.m_StartFacing.m_x, worldData.m_StartFacing.m_y, worldData.m_StartFacing.m_z);

	// portals
	m_portals.Resize(worldData.m_Portal.size());
	for (unsigned int index = 0, count = worldData.m_Portal.size(); index < count; ++index)
	{
		m_portals[index].m_sector = SData::GetEntryById(worldData.m_Sector, worldData.m_Portal[index].m_Sector);
		Copy(m_portals[index].m_xaxis, worldData.m_Portal[index].m_XAxis);
		Copy(m_portals[index].m_yaxis, worldData.m_Portal[index].m_YAxis);
		Copy(m_portals[index].m_zaxis, worldData.m_Portal[index].m_ZAxis);
		Copy(m_portals[index].m_waxis, worldData.m_Portal[index].m_WAxis);
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

	// sectors
	m_sectors.Resize(worldData.m_Sector.size());
	for (unsigned int sectorIndex = 0, sectorCount = worldData.m_Sector.size(); sectorIndex < sectorCount; ++sectorIndex)
		LoadSector(m_sectors[sectorIndex], worldData.m_Sector[sectorIndex], worldData.m_Material, worldData.m_Portal);

	// calculate the texture indices of our textures
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

	return true;
}