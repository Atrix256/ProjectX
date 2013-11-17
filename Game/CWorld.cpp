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
		m_boxes[index].m_materialIndex = 0;
		for(unsigned int matIndex = 0, matCount = worldData.m_Material.size(); matIndex < matCount; ++matIndex)
		{
			if (!stricmp(worldData.m_Box[index].m_Material.c_str(), worldData.m_Material[matIndex].m_id.c_str()))
			{
				m_boxes[index].m_materialIndex = matIndex;
				break;
			}
		}
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
		m_spheres[index].m_materialIndex = 0;
		for(unsigned int matIndex = 0, matCount = worldData.m_Material.size(); matIndex < matCount; ++matIndex)
		{
			if (!stricmp(worldData.m_Sphere[index].m_Material.c_str(), worldData.m_Material[matIndex].m_id.c_str()))
			{
				m_spheres[index].m_materialIndex = matIndex;
				break;
			}
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
	worldShared.m_numMaterials = m_materials.Count();

	return true;
}