/*==================================================================================================

CWorld.cpp

This class holds all information about the world

==================================================================================================*/

#include "CWorld.h"

#include "DataSchemas/DataSchemasXML.h"

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
	// clear the existing world data
	m_ambientLight[0] = 0.0f;
	m_ambientLight[1] = 0.0f;
	m_ambientLight[2] = 0.0f;
	m_pointLights.Clear();
	m_materials.Clear();
	m_boxes.Clear();
	m_spheres.Clear();

	SData_World worldData;
	if (!DataSchemasXML::Load(worldData, worldFileName, "World"))
		return false;

	// ambient light
	Copy(m_ambientLight, worldData.m_AmbientLight);

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
		Copy(m_materials[index].m_diffuseColorAndAmount, worldData.m_Material[index].m_DiffuseColor, worldData.m_Material[index].m_DiffuseAmount);
		Copy(m_materials[index].m_specularColorAndAmount, worldData.m_Material[index].m_SpecularColor, worldData.m_Material[index].m_SpecularAmount);
		Copy(m_materials[index].m_emissiveColor, worldData.m_Material[index].m_EmissiveColor);
		m_materials[index].m_reflectionAmount = worldData.m_Material[index].m_ReflectionAmount;
		m_materials[index].m_refractionAmount = worldData.m_Material[index].m_RefractionAmount;
		m_materials[index].m_refractionIndex = worldData.m_Material[index].m_RefractionIndex;
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

	return true;
}