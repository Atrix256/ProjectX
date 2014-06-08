/*==================================================================================================

CWorld.cpp

This class holds all information about the world

==================================================================================================*/

#include "Platform\CDirectx.h"

#include "CWorld.h"

#include "DataSchemas/DataSchemasXML.h"

#include "CGame.h"
#include "MatrixMath.h"
#include "CCamera.h"
#include "Platform/OS.h"

#include <algorithm>

static const unsigned int c_defaultMaterial = -1; // the default "error" material
static const unsigned int c_defaultPortal = -1;
static const unsigned int c_defaultModel = -1;
static const unsigned int c_defaultSector = -1;

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
void CWorld::AddTriangle (
	const SData_Vec3 &sa,
	const SData_Vec3 &sb,
	const SData_Vec3 &sc,
	const SData_Vec2 &ta,
	const SData_Vec2 &tb,
	const SData_Vec2 &tc,
	const struct SData_Vec3 &normal,
	const struct SData_Vec3 &tangent,
	const struct SData_Vec3 &bitangent
) {
	SModelTriangle &triangle = m_modelTriangles.AddOne();
	triangle.m_objectId = m_nextObjectId++;

	// calculate pre-calculated info for triangle
	float3 a,b,c;
	Copy(a, sa);
	Copy(b, sb);
	Copy(c, sc);

	float3 norm;
	Copy(norm, normal);
	norm = normalize(norm);
	triangle.m_plane   = plane(normalize(norm), a);
	triangle.m_planeBC = plane(normalize(cross(norm, c-b)), b);
	triangle.m_planeCA = plane(normalize(cross(norm, a-c)), c);

	float bc = 1.0f / (dot(a, normalize(cross(norm, c-b))) - triangle.m_planeBC.s[3]);
	float ca = 1.0f / (dot(b, normalize(cross(norm, a-c))) - triangle.m_planeCA.s[3]);

	triangle.m_planeBC.s[0] *= bc;
	triangle.m_planeBC.s[1] *= bc;
	triangle.m_planeBC.s[2] *= bc;
	triangle.m_planeBC.s[3] *= bc;

	triangle.m_planeCA.s[0] *= ca;
	triangle.m_planeCA.s[1] *= ca;
	triangle.m_planeCA.s[2] *= ca;
	triangle.m_planeCA.s[3] *= ca;

	Copy(triangle.m_textureA, ta);
	Copy(triangle.m_textureB, tb);
	Copy(triangle.m_textureC, tc);

	Copy(triangle.m_tangent, tangent);
	Copy(triangle.m_bitangent, bitangent);

	// calculate the quadrant flags by noting which half spaces this triangle exists in
	triangle.m_halfSpaceFlags = 0;
	triangle.m_halfSpaceFlags |= sa.m_y > 0.0f ? e_halfSpacePosY : e_halfSpaceNegY;
	triangle.m_halfSpaceFlags |= sb.m_y > 0.0f ? e_halfSpacePosY : e_halfSpaceNegY;
	triangle.m_halfSpaceFlags |= sc.m_y > 0.0f ? e_halfSpacePosY : e_halfSpaceNegY;
}

//-----------------------------------------------------------------------------
unsigned int CWorld::AddMaterial (const struct SData_Material &materialSource, const char *path)
{
	SMaterial &material = m_materials.AddOne();
	Copy(material.m_diffuseColor, materialSource.m_DiffuseColor);
	Copy(material.m_specularColorAndPower, materialSource.m_SpecularColor, materialSource.m_SpecularPower);
	Copy(material.m_emissiveColor, materialSource.m_EmissiveColor);
	Copy(material.m_reflectionColor, materialSource.m_ReflectionColor);
	Copy(material.m_refractionColor, materialSource.m_RefractionColor);

	material.m_rayInteraction =
		lengthsq(material.m_reflectionColor) > 0 ? e_rayInteractionReflect : (lengthsq(material.m_refractionColor) > 0 ? e_rayInteractionRefract : e_rayInteractionNone );

	material.m_refractionIndex = materialSource.m_RefractionIndex;

	Copy(material.m_absorbance, materialSource.m_Absorbance);

	// convert absorbance from absorbance per centimeter to absorbance per world unit (meters)
	material.m_absorbance[0] *= 100.0f;
	material.m_absorbance[1] *= 100.0f;
	material.m_absorbance[2] *= 100.0f;

	material.m_diffuseTextureIndex = 0.0f;
	if (materialSource.m_DiffuseTexture.length() > 0)
	{
		std::string texturePath = path;
		texturePath += materialSource.m_DiffuseTexture.c_str();
		material.m_diffuseTextureIndex = (float)CDirectX::TextureManager().GetOrLoad(texturePath.c_str());
	}

	material.m_normalTextureIndex = 0.0f;
	if (materialSource.m_NormalTexture.length() > 0)
	{
		std::string texturePath = path;
		texturePath += materialSource.m_NormalTexture.c_str();
		material.m_normalTextureIndex = (float)CDirectX::TextureManager().GetOrLoad(texturePath.c_str());
	}

	material.m_emissiveTextureIndex = 0.0f;
	if (materialSource.m_EmissiveTexture.length() > 0)
	{
		std::string texturePath = path;
		texturePath += materialSource.m_EmissiveTexture.c_str();
		material.m_emissiveTextureIndex = (float)CDirectX::TextureManager().GetOrLoad(texturePath.c_str());
	}

	return m_materials.Count() - 1;
}

//-----------------------------------------------------------------------------
void CWorld::AddDebugMaterial ()
{
	SData_Material material;
	material.SetDefault();
	material.m_EmissiveColor.m_x = 1.0f;
	material.m_EmissiveColor.m_y = 1.0f;
	material.m_EmissiveColor.m_z = 1.0f;
	material.m_EmissiveTexture="Art/error.png";
	AddMaterial(material);
}

//-----------------------------------------------------------------------------
void CWorld::AddModel (const struct SData_Model &modelSource)
{
	SNamedModel namedModel;

	// set the starting object index and copy the id
	namedModel.m_startObjectIndex = m_modelObjects.Count();
	namedModel.m_id = modelSource.m_id;
		
	// init bounding sphere data
	namedModel.m_farthestPointFromOrigin[0] = 0.0f;
	namedModel.m_farthestPointFromOrigin[1] = 0.0f;
	namedModel.m_farthestPointFromOrigin[2] = 0.0f;

	SData_XMDFILE modelData;
	if (DataSchemasXML::Load(modelData, modelSource.m_FileName.c_str(), "model"))
	{
		// get the path that all textures etc are based on
		std::string basePath;
		bool result = OS::GetAbsolutePath(modelSource.m_FileName.c_str(), basePath);
		Assert_(result == true);

		// remember which vertex is farthest from the origin
		CalculateModelFarthestPoint(modelData, namedModel.m_farthestPointFromOrigin);

		// create the objects and triangles and such
		for (unsigned int objectIndex = 0, objectCount = modelData.m_object.size(); objectIndex < objectCount; ++objectIndex) {

			// TODO: log error instead? what if there are zero and we try to index slot 0?
			AssertI_(modelData.m_object[objectIndex].m_material.size() == 1, modelData.m_object[objectIndex].m_material.size());

			// add an object
			SModelObject &modelobject = m_modelObjects.AddOne();
			modelobject.m_castsShadows = modelData.m_object[objectIndex].m_CastShadows;
			modelobject.m_materialIndex = AddMaterial(modelData.m_object[objectIndex].m_material[0], basePath.c_str());
			modelobject.m_startTriangleIndex = m_modelTriangles.Count();

			SData_object &object = modelData.m_object[objectIndex];
			for (unsigned int faceIndex = 0, faceCount = object.m_face.size(); faceIndex < faceCount; ++faceIndex) {
				SData_face &face = object.m_face[faceIndex];
				AssertI_(face.m_vert.size() == 3, face.m_vert.size()); // TODO: log error instead?
				AddTriangle(
					face.m_vert[0].m_pos,
					face.m_vert[1].m_pos,
					face.m_vert[2].m_pos,
					face.m_vert[0].m_uv,
					face.m_vert[1].m_uv,
					face.m_vert[2].m_uv,
					face.m_vert[0].m_normal,
					face.m_vert[0].m_tangent,
					face.m_vert[0].m_bitangent
				);
			}

			modelobject.m_stopTriangleIndex = m_modelTriangles.Count();

			// sort the triangles from m_startTriangleIndex to m_stopTriangleIndex, based on their 
			// y axis (negative only, mixed, positive only) so that we can set the mix start and mix end
			// indices.  This way, when rendering, if the line segment is only in pos, or only in neg,
			// we can limit the triangles we test.
			SortTrianglesByHalfSpace(modelobject);
		}

		namedModel.m_stopObjectIndex = m_modelObjects.Count();
		m_namedModels.push_back(namedModel);
	}
}

//-----------------------------------------------------------------------------
static bool TriangleHalfSpaceSortFunc (const SModelTriangle &a, const SModelTriangle &b)
{
	// sort order is e_halfSpaceNegY, then mixed, then e_halfSpacePosY
	int orderA = 0;
	switch (a.m_halfSpaceFlags)
	{
		case e_halfSpaceNegY: orderA = 0; break;
		case e_halfSpaceNegY | e_halfSpacePosY: orderA = 1; break;
		case e_halfSpacePosY: orderA = 2; break;
		default: Assert_(false);
	}

	int orderB = 0;
	switch (b.m_halfSpaceFlags)
	{
		case e_halfSpaceNegY: orderB = 0; break;
		case e_halfSpaceNegY | e_halfSpacePosY: orderB = 1; break;
		case e_halfSpacePosY: orderB = 2; break;
		default: Assert_(false);
	}

	// sort by the order described above
	return orderA < orderB;
}

//-----------------------------------------------------------------------------
void CWorld::SortTrianglesByHalfSpace (SModelObject &object)
{
	// sort the triangles by the y axis half space flags
	std::sort(
		&m_modelTriangles[object.m_startTriangleIndex],
		&m_modelTriangles[object.m_startTriangleIndex] + (object.m_stopTriangleIndex - object.m_startTriangleIndex),
		TriangleHalfSpaceSortFunc
	);

	// find where any e_halfSpacePosY starts, and store it in m_mixStartTriangleIndex
	for (object.m_mixStartTriangleIndex = object.m_startTriangleIndex;  object.m_mixStartTriangleIndex < object.m_stopTriangleIndex; ++object.m_mixStartTriangleIndex)
	{
		if ((m_modelTriangles[object.m_mixStartTriangleIndex].m_halfSpaceFlags & e_halfSpacePosY) != 0)
			break;
	}

	// find where all e_halfSpaceNegY ends and store it in m_mixStopTriangleIndex
	for (object.m_mixStopTriangleIndex = object.m_startTriangleIndex;  object.m_mixStopTriangleIndex < object.m_stopTriangleIndex; ++object.m_mixStopTriangleIndex)
	{
		if ((m_modelTriangles[object.m_mixStopTriangleIndex].m_halfSpaceFlags & e_halfSpaceNegY) == 0)
			break;
	}
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
		Copy(sphere.m_textureOffset, sphereSource.m_TextureOffset);

		// set the material index
		sphere.m_materialIndex = SData::GetEntryById(materials, sphereSource.m_Material, c_defaultMaterial) + 1;

		// set the portal index
		sphere.m_portalIndex = SData::GetEntryById(portals, sphereSource.m_Portal, c_defaultPortal);
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

		// point light params
		Copy(light.m_color, lightSource.m_Color);
		Copy(light.m_position, lightSource.m_Position);
		light.m_attenuationConstDistDistsq[0] = lightSource.m_AttenuationConstant;
		light.m_attenuationConstDistDistsq[1] = lightSource.m_AttenuationDistance;
		light.m_attenuationConstDistDistsq[2] = lightSource.m_AttenuationDistanceSquared;

		// spot light params
		Copy(light.m_spotLightReverseDir, lightSource.m_ConeDirection);
		Normalize(light.m_spotLightReverseDir);
		light.m_spotLightReverseDir[0] *= -1.0f;
		light.m_spotLightReverseDir[1] *= -1.0f;
		light.m_spotLightReverseDir[2] *= -1.0f;

		light.m_spotLightFalloffFactor = lightSource.m_ConeFalloffFactor;
		light.m_spotLightcosThetaOver2 = cos(((lightSource.m_ConeAngle - + lightSource.m_ConeAttenuationAngle) * 3.14f / 180.0f) / 2.0f);
		light.m_spotLightcosPhiOver2 = cos((lightSource.m_ConeAngle * 3.14f / 180.0f) / 2.0f);
	}
	sector.m_staticLightStopIndex = m_pointLights.Count();
}

//-----------------------------------------------------------------------------
void CWorld::LoadSectorModelInstances (
	SSector &sector,
	struct SData_Sector &sectorSource,
	std::vector<struct SData_Material> &materials,
	std::vector<struct SData_Portal> &portals
) {
	sector.m_staticModelStartIndex = m_modelInstances.Count();

	for (unsigned int index = 0, count = sectorSource.m_ModelInstance.size(); index < count; ++index)
	{
		SData_ModelInstance &model = sectorSource.m_ModelInstance[index];
		unsigned int modelIndex = SData::GetEntryById(m_namedModels, model.m_ModelId, c_defaultModel);
		if (modelIndex != -1)
		{
			const SNamedModel &namedModel = m_namedModels[modelIndex];
			SModelInstance &modelInstance = m_modelInstances.AddOne();

			// copy the start and stop object index
			modelInstance.m_startObjectIndex = namedModel.m_startObjectIndex;
			modelInstance.m_stopObjectIndex = namedModel.m_stopObjectIndex;

			// calculate the bounding sphere of this instance
			modelInstance.m_boundingSphere.s[0] = model.m_Position.m_x;
			modelInstance.m_boundingSphere.s[1] = model.m_Position.m_y;
			modelInstance.m_boundingSphere.s[2] = model.m_Position.m_z;
			modelInstance.m_boundingSphere.s[3] = sqrtf(lengthsq(namedModel.m_farthestPointFromOrigin)) * model.m_Scale;
			modelInstance.m_scale = model.m_Scale;

			// set material override if there is one
			if (model.m_MaterialOverride.length() > 0)
				modelInstance.m_materialOverride = SData::GetEntryById(materials, model.m_MaterialOverride, c_defaultMaterial) + 1;
			else
				modelInstance.m_materialOverride = -1;
			
			// set the portal if there is one
			modelInstance.m_portalIndex = SData::GetEntryById(portals, model.m_Portal, c_defaultPortal);

			// calculate model to world - translate, scale, rotate
			{
				MatrixIdentity(modelInstance.m_modelToWorldX, modelInstance.m_modelToWorldY, modelInstance.m_modelToWorldZ, modelInstance.m_modelToWorldW);

				// make the translation matrix
				cl_float4 transX;
				cl_float4 transY;
				cl_float4 transZ;
				cl_float4 transW;
				float3 translation;
				Copy(translation, model.m_Position);
				MatrixTranslation(transX, transY, transZ, transW, translation);

				// make the scale matrix
				cl_float4 scaleX;
				cl_float4 scaleY;
				cl_float4 scaleZ;
				cl_float4 scaleW;
				MatrixScale(scaleX, scaleY, scaleZ, scaleW, model.m_Scale);

				// make the rotation matrix
				cl_float4 rotX;
				cl_float4 rotY;
				cl_float4 rotZ;
				cl_float4 rotW;
				MatrixRotation(rotX, rotY, rotZ, rotW,
					DegreesToRadians(model.m_Rotation.m_x),
					DegreesToRadians(model.m_Rotation.m_y),
					DegreesToRadians(model.m_Rotation.m_z));

				TransformMatrixByMatrix(
					modelInstance.m_modelToWorldX,
					modelInstance.m_modelToWorldY,
					modelInstance.m_modelToWorldZ,
					modelInstance.m_modelToWorldW,
					rotX,
					rotY,
					rotZ,
					rotW
				);

				TransformMatrixByMatrix(
					modelInstance.m_modelToWorldX,
					modelInstance.m_modelToWorldY,
					modelInstance.m_modelToWorldZ,
					modelInstance.m_modelToWorldW,
					scaleX,
					scaleY,
					scaleZ,
					scaleW
				);

				TransformMatrixByMatrix(
					modelInstance.m_modelToWorldX,
					modelInstance.m_modelToWorldY,
					modelInstance.m_modelToWorldZ,
					modelInstance.m_modelToWorldW,
					transX,
					transY,
					transZ,
					transW
				);
			}

			// calculate world to model - rotate, scale, translate
			{
				MatrixIdentity(modelInstance.m_worldToModelX, modelInstance.m_worldToModelY, modelInstance.m_worldToModelZ, modelInstance.m_worldToModelW);

				// make the translation matrix
				cl_float4 transX;
				cl_float4 transY;
				cl_float4 transZ;
				cl_float4 transW;
				float3 translation;
				Copy(translation, model.m_Position);
				translation *= -1.0f;
				MatrixTranslation(transX, transY, transZ, transW, translation);

				// make the scale matrix
				cl_float4 scaleX;
				cl_float4 scaleY;
				cl_float4 scaleZ;
				cl_float4 scaleW;
				MatrixScale(scaleX, scaleY, scaleZ, scaleW, 1.0f / model.m_Scale);

				// make the rotation matrix
				cl_float4 rotX;
				cl_float4 rotY;
				cl_float4 rotZ;
				cl_float4 rotW;
				MatrixUnrotation(rotX, rotY, rotZ, rotW,
					DegreesToRadians(model.m_Rotation.m_x),
					DegreesToRadians(model.m_Rotation.m_y),
					DegreesToRadians(model.m_Rotation.m_z));

				TransformMatrixByMatrix(
					modelInstance.m_worldToModelX,
					modelInstance.m_worldToModelY,
					modelInstance.m_worldToModelZ,
					modelInstance.m_worldToModelW,
					transX,
					transY,
					transZ,
					transW
				);

				TransformMatrixByMatrix(
					modelInstance.m_worldToModelX,
					modelInstance.m_worldToModelY,
					modelInstance.m_worldToModelZ,
					modelInstance.m_worldToModelW,
					scaleX,
					scaleY,
					scaleZ,
					scaleW
				);

				TransformMatrixByMatrix(
					modelInstance.m_worldToModelX,
					modelInstance.m_worldToModelY,
					modelInstance.m_worldToModelZ,
					modelInstance.m_worldToModelW,
					rotX,
					rotY,
					rotZ,
					rotW
				);
			}

			// todo: remove, this is just for testing
			cl_float4 testX;
			cl_float4 testY;
			cl_float4 testZ;
			cl_float4 testW;

			TransformMatrixByMatrix(
				testX,
				testY,
				testZ,
				testW,
				modelInstance.m_modelToWorldX,
				modelInstance.m_modelToWorldY,
				modelInstance.m_modelToWorldZ,
				modelInstance.m_modelToWorldW,
				modelInstance.m_worldToModelX,
				modelInstance.m_worldToModelY,
				modelInstance.m_worldToModelZ,
				modelInstance.m_worldToModelW
			);

			int ijkl = 0;
		}
	}

	sector.m_staticModelStopIndex = m_modelInstances.Count();
}

//-----------------------------------------------------------------------------
void CWorld::CalculateModelFarthestPoint (
	const struct SData_XMDFILE &modelData,
	float3 &point
)
{
	point[0] = 0.0f;
	point[1] = 0.0f;
	point[2] = 0.0f;
	float radiusSq = 0.0f;

	// find the point that is farthest from the origin (0,0,0)
	for (unsigned int objectIndex = 0, objectCount = modelData.m_object.size(); objectIndex < objectCount; ++objectIndex)
	{
		const SData_object &object = modelData.m_object[objectIndex];
		for (unsigned int faceIndex = 0, faceCount = object.m_face.size(); faceIndex < faceCount; ++faceIndex)
		{
			const SData_face &face = object.m_face[faceIndex];
			for (unsigned int vertIndex = 0, vertCount = face.m_vert.size(); vertIndex < vertCount; ++vertIndex)
			{
				float3 pos;
				Copy(pos, face.m_vert[vertIndex].m_pos);
				float posRadius = lengthsq(pos);
				if (posRadius > radiusSq)
				{
					point = pos;
					radiusSq = posRadius;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CWorld::HandleSectorConnectTos (
	unsigned int sectorIndex,
	const std::vector<struct SData_Sector> &sectorsSource
)
{
	const SData_Sector &sectorSource = sectorsSource[sectorIndex];
	for (unsigned int planeIndex = 0; planeIndex < SSECTOR_NUMPLANES; ++planeIndex)
	{
		const SData_SectorPlane &sectorPlaneSource = sectorSource.m_SectorPlane[planeIndex];

		// if no connect to sector specified, or none found, bail
		unsigned int connectToSectorIndex = SData::GetEntryById(sectorsSource, sectorPlaneSource.m_ConnectToSector, c_defaultSector);
		if (connectToSectorIndex >= m_sectors.Count())
			continue;

		// if invalid connect to sector plane specified, bail
		if (sectorPlaneSource.m_ConnectToSectorPlane >= SSECTOR_NUMPLANES)
			continue;

		// now we have a sector to connect to and the plane to connect it to, so let's make it happen
		cl_float2 offset;
		cl_float4 portalWindow;
		float3 connectToPosition;
		Copy(portalWindow, sectorPlaneSource.m_PortalWindow);
		Copy(offset, sectorPlaneSource.m_ConnectToSectorOffset);
		Copy(connectToPosition, sectorPlaneSource.m_ConnectToPosition);
		ConnectSectors(
			sectorIndex,
			planeIndex,
			connectToSectorIndex,
			sectorPlaneSource.m_ConnectToSectorPlane,
			offset,
			portalWindow,
			sectorPlaneSource.m_ConnectToSetPosition,
			connectToPosition);
	}
}

//-----------------------------------------------------------------------------
void CWorld::ConnectSectors (
	unsigned int sectorIndex,
	unsigned int planeIndex,
	unsigned int destSectorIndex,
	unsigned int destPlaneIndex,
	const cl_float2 &offset,
	const cl_float4 &portalWindow,
	bool setPosition,
	const float3 &position
)
{
	SSector &sector = m_sectors[sectorIndex];
	cl_float4 srcSectorXaxis;
	cl_float4 srcSectorYaxis;
	cl_float4 srcSectorZaxis;
	cl_float4 srcSectorWaxis;

	GetSectorPlaneInverseTransformationMatrix(
		sector,
		planeIndex,
		srcSectorXaxis,
		srcSectorYaxis,
		srcSectorZaxis,
		srcSectorWaxis
	);

	SSector &destSector = m_sectors[destSectorIndex];
	cl_float4 destSectorXaxis;
	cl_float4 destSectorYaxis;
	cl_float4 destSectorZaxis;
	cl_float4 destSectorWaxis;

	GetSectorPlaneTransformationMatrix(
		destSector,
		destPlaneIndex,
		destSectorXaxis,
		destSectorYaxis,
		destSectorZaxis,
		destSectorWaxis
	);

	// add the offset to the destination sector W axis
	destSectorWaxis.s[0] += destSectorXaxis.s[0] * offset.s[0] + destSectorYaxis.s[0] * offset.s[1];
	destSectorWaxis.s[1] += destSectorXaxis.s[1] * offset.s[0] + destSectorYaxis.s[1] * offset.s[1];
	destSectorWaxis.s[2] += destSectorXaxis.s[2] * offset.s[0] + destSectorYaxis.s[2] * offset.s[1];

	// to convert points & vectors from the specified sector & sector wall to the other specified sector & sector wall
	// first you untransform by the first (source) wall, to bring it back to being "untransformed", and then you
	// transform it by the destination walls transform to bring it into that wall's space.
	cl_float4 portalXaxis;
	cl_float4 portalYaxis;
	cl_float4 portalZaxis;
	cl_float4 portalWaxis;
	TransformMatrixByMatrix(
		portalXaxis, portalYaxis, portalZaxis, portalWaxis,
		srcSectorXaxis, srcSectorYaxis, srcSectorZaxis, srcSectorWaxis,
		destSectorXaxis, destSectorYaxis, destSectorZaxis, destSectorWaxis
	);

	// make a new portal with this information
	SPortal &newPortal = m_portals.AddOne();
	newPortal.m_sector = destSectorIndex;
	newPortal.m_xaxis = portalXaxis;
	newPortal.m_yaxis = portalYaxis;
	newPortal.m_zaxis = portalZaxis;
	newPortal.m_waxis = portalWaxis;
	newPortal.m_setPosition = setPosition;
	newPortal.m_position = position;

	// calculate our portal window by taking the most restrictive values between
	// the translated src and destination dimensions, and the portal window passed in
	float srcMinX, srcMinY, srcMaxX, srcMaxY;
	GetSectorPlaneDimenions(sector, planeIndex, srcMinX, srcMinY, srcMaxX, srcMaxY);

	float destMinX, destMinY, destMaxX, destMaxY;
	GetSectorPlaneDimenions(destSector, destPlaneIndex, destMinX, destMinY, destMaxX, destMaxY);

	// apply the offset passed in
	destMinX += offset.s[0];
	destMaxX += offset.s[0];
	destMinY -= offset.s[1];
	destMaxY -= offset.s[1];

	sector.m_planes[planeIndex].m_portalWindow.s[0] = (srcMinX >= destMinX) ? srcMinX : destMinX;
	sector.m_planes[planeIndex].m_portalWindow.s[1] = (srcMinY >= destMinY) ? srcMinY : destMinY;
	sector.m_planes[planeIndex].m_portalWindow.s[2] = (srcMaxX <= destMaxX) ? srcMaxX : destMaxX;
	sector.m_planes[planeIndex].m_portalWindow.s[3] = (srcMaxY <= destMaxY) ? srcMaxY : destMaxY;

	if (portalWindow.s[0] > sector.m_planes[planeIndex].m_portalWindow.s[0])
		sector.m_planes[planeIndex].m_portalWindow.s[0] = portalWindow.s[0];

	if (portalWindow.s[1] > sector.m_planes[planeIndex].m_portalWindow.s[1])
		sector.m_planes[planeIndex].m_portalWindow.s[1] = portalWindow.s[1];

	if (portalWindow.s[2] < sector.m_planes[planeIndex].m_portalWindow.s[2])
		sector.m_planes[planeIndex].m_portalWindow.s[2] = portalWindow.s[2];

	if (portalWindow.s[3] < sector.m_planes[planeIndex].m_portalWindow.s[3])
		sector.m_planes[planeIndex].m_portalWindow.s[3] = portalWindow.s[3];

	// make the sector plane use this new portal
	sector.m_planes[planeIndex].m_portalIndex = m_portals.Count() - 1;
}

//-----------------------------------------------------------------------------
void CWorld::GetSectorPlaneTransformationMatrix (
	const SSector &sector,
	unsigned int planeIndex,
	cl_float4 &xAxis,
	cl_float4 &yAxis,
	cl_float4 &zAxis,
	cl_float4 &wAxis
)
{
	const cl_float4 globalAxisX = {1.0f, 0.0f, 0.0f, 0.0f};
	const cl_float4 globalAxisY = {0.0f, 1.0f, 0.0f, 0.0f};
	const cl_float4 globalAxisZ = {0.0f, 0.0f, 1.0f, 0.0f};

	const cl_float4 globalAxisNegX = {-1.0f, 0.0f, 0.0f, 0.0f};
	const cl_float4 globalAxisNegY = {0.0f, -1.0f, 0.0f, 0.0f};
	const cl_float4 globalAxisNegZ = {0.0f, 0.0f, -1.0f, 0.0f};

	const cl_float4 zero = {0.0f, 0.0f, 0.0f, 0.0f};
	xAxis = zero;
	yAxis = zero;
	zAxis = zero;
	wAxis = zero;
	wAxis.s[3] = 1.0f;

	switch(planeIndex)
	{
		case 0: // positive x
		{
			xAxis = globalAxisNegZ;
			yAxis = globalAxisY;
			zAxis = globalAxisNegX;
			wAxis.s[0] = sector.m_halfDims[0];
			break;
		}
		case 1: // negative x
		{
			xAxis = globalAxisZ;
			yAxis = globalAxisY;
			zAxis = globalAxisX;
			wAxis.s[0] = -sector.m_halfDims[0];
			break;
		}
		case 2: // positive y
		{
			xAxis = globalAxisX;
			yAxis = globalAxisNegZ;
			zAxis = globalAxisNegY;
			wAxis.s[1] = sector.m_halfDims[1];
			break;
		}
		case 3: // negative y
		{
			xAxis = globalAxisNegX;
			yAxis = globalAxisZ;
			zAxis = globalAxisY;
			wAxis.s[1] = -sector.m_halfDims[1];
			break;
		}
		case 4: // positive z
		{
			xAxis = globalAxisX;
			yAxis = globalAxisY;
			zAxis = globalAxisNegZ;
			wAxis.s[2] = sector.m_halfDims[2];
			break;
		}
		case 5: // negative z
		{
			xAxis = globalAxisNegX;
			yAxis = globalAxisY;
			zAxis = globalAxisZ;
			wAxis.s[2] = -sector.m_halfDims[2];
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void CWorld::GetSectorPlaneInverseTransformationMatrix (
	const SSector &sector,
	unsigned int planeIndex,
	cl_float4 &xAxis,
	cl_float4 &yAxis,
	cl_float4 &zAxis,
	cl_float4 &wAxis
)
{
	GetSectorPlaneTransformationMatrix(sector, planeIndex, xAxis, yAxis, zAxis, wAxis);

	// rotate around the y axis of the plane by 180 degrees because we are looking at a wall
	// and getting the basis vectors for the normal to the wall pointing back at us, but
	// what we want is that normal pointing out the back of the wall

	// if this is an XZ plane, rotate 180 degrees around the y axis
	if (planeIndex / 2 != 1)
	{
		cl_float4 rot180x = {-1.0f,  0.0f,  0.0f,  0.0f};
		cl_float4 rot180y = { 0.0f,  1.0f,  0.0f,  0.0f};
		cl_float4 rot180z = { 0.0f,  0.0f, -1.0f,  0.0f};
		cl_float4 rot180w = { 0.0f,  0.0f,  0.0f,  1.0f};

		TransformMatrixByMatrix(xAxis,yAxis,zAxis,wAxis,rot180x,rot180y,rot180z,rot180w);
	}
	// is if it's a Y plane, rotate 180 degrees around the z axis
	else
	{
		cl_float4 rot180x = {-1.0f,  0.0f,  0.0f,  0.0f};
		cl_float4 rot180y = { 0.0f, -1.0f,  0.0f,  0.0f};
		cl_float4 rot180z = { 0.0f,  0.0f, -1.0f,  0.0f};
		cl_float4 rot180w = { 0.0f,  0.0f,  0.0f,  1.0f};

		TransformMatrixByMatrix(xAxis,yAxis,zAxis,wAxis,rot180x,rot180y,rot180z,rot180w);
	}


	// We know that the 3x3 of the transformation matrix is just a rotation
	// matrix (made from basis vectors) and that the inverse of a rotation matrix
	// is the transpose.
	// We also know that the remainder of the 4x4 matrix is just a translation,
	// so it's also invertable.
	//
	// To invert a rotation then translation, we would want to do the negative
	// translation and then the negative rotation.  We can't do that with a naive
	// matrix, so we have to make an inverse matrix by doing the below:
	//
	// inv(A) = [ inv(M)   -inv(M) * b ]
    //          [   0            1     ]
	//
	// A - The matrix we want to invert
	// M - the 3x3 rotation matrix
	// b - the translation of the matrix
	//
	// Sources:
	// http://negativeprobability.blogspot.com/2011/11/affine-transformations-and-their.html
	// http://stackoverflow.com/questions/2624422/efficient-4x4-matrix-inverse-affine-transform

	// invert the 3x3 rotation matrix
	float3 invXAxis;
	float3 invYAxis;
	float3 invZAxis;

	invXAxis[0] = xAxis.s[0];
	invXAxis[1] = yAxis.s[0];
	invXAxis[2] = zAxis.s[0];

	invYAxis[0] = xAxis.s[1];
	invYAxis[1] = yAxis.s[1];
	invYAxis[2] = zAxis.s[1];

	invZAxis[0] = xAxis.s[2];
	invZAxis[1] = yAxis.s[2];
	invZAxis[2] = zAxis.s[2];

	// multiply our translation by the inverted 3x3 rotation matrix and put the transformed
	// translation back into the w axis
	float3 translation;
	translation[0] = wAxis.s[0];
	translation[1] = wAxis.s[1];
	translation[2] = wAxis.s[2];

	TransformPointOrVectorByMatrix(translation, invXAxis, invYAxis, invZAxis);

	wAxis.s[0] = translation[0];
	wAxis.s[1] = translation[1];
	wAxis.s[2] = translation[2];

	// put the inverted 3x3 rotation matrix back in
	xAxis.s[0] = invXAxis[0];
	xAxis.s[1] = invXAxis[1];
	xAxis.s[2] = invXAxis[2];

	yAxis.s[0] = invYAxis[0];
	yAxis.s[1] = invYAxis[1];
	yAxis.s[2] = invYAxis[2];

	zAxis.s[0] = invZAxis[0];
	zAxis.s[1] = invZAxis[1];
	zAxis.s[2] = invZAxis[2];
}

//-----------------------------------------------------------------------------
void CWorld::GetSectorPlaneDimenions (
	const SSector &sector,
	unsigned int planeIndex,
	float &minX,
	float &minY,
	float &maxX,
	float &maxY
) {
	switch (planeIndex / 2)
	{
		// +/- X Walls ->  Z,Y
		case 0:
		{
			minX = -sector.m_halfDims[2];
			minY = -sector.m_halfDims[1];
			maxX = minX * -1;
			maxY = minY * -1;
			break;
		}
		// +/- Y Walls ->  X,Z
		case 1:
		{
			minX = -sector.m_halfDims[0];
			minY = -sector.m_halfDims[2];
			maxX = minX * -1;
			maxY = minY * -1;
			break;
		}
		// +/- Z Walls ->  X,Y
		case 2:
		{
			minX = -sector.m_halfDims[0];
			minY = -sector.m_halfDims[1];
			maxX = minX * -1;
			maxY = minY * -1;
			break;
		}
	}
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
	Copy(sector.m_fogColorAndFactor, sectorSource.m_FogColor, sectorSource.m_FogDensityFactor);
	sector.m_fogFactorMax = sectorSource.m_FogFactorMax;
	Copy(sector.m_fogPlane, sectorSource.m_FogPlane);
	Normalize(sector.m_fogPlane);
	Copy(sector.m_halfDims, sectorSource.m_Dimensions);
	sector.m_halfDims /= 2.0f;

	if (sectorSource.m_FogConstantDensity)
		sector.m_fogMode = sectorSource.m_FogFactorMax > 0.0f ? e_fogConstantDensity : e_fogNone;
	else
		sector.m_fogMode = (sectorSource.m_FogFactorMax > 0.0f && sectorSource.m_FogDensityFactor > 0.0f) ? e_fogLinearDensity : e_fogNone;

	// load the sector planes data
	for (unsigned int planeIndex = 0; planeIndex < SSECTOR_NUMPLANES; ++planeIndex)
	{
		SSectorPlane &plane = sector.m_planes[planeIndex];
		SData_SectorPlane &planeSource = sectorSource.m_SectorPlane[planeIndex];
		plane.m_objectId = m_nextObjectId++;

		Copy(plane.m_UAxis, planeSource.m_UAxis);
		Copy(plane.m_textureScale, planeSource.m_TextureScale);
		Copy(plane.m_textureOffset, planeSource.m_TextureOffset);

		// copy the portal window
		Copy(plane.m_portalWindow, planeSource.m_PortalWindow);

		// make sure the Uaxis is normalized
		Normalize(plane.m_UAxis);

		// set the material index
		plane.m_materialIndex = SData::GetEntryById(materials, planeSource.m_Material, c_defaultMaterial) + 1;

		// set the portal index
		plane.m_portalIndex = SData::GetEntryById(portals, planeSource.m_Portal, c_defaultPortal);
	}

	LoadSectorPointLights(sector, sectorSource, materials, portals);
	LoadSectorSpheres(sector, sectorSource, materials, portals);
	LoadSectorModelInstances(sector, sectorSource, materials, portals);
}

//-----------------------------------------------------------------------------
bool CWorld::Load (const char *worldFileName)
{
	SData_World worldData;
	if (!DataSchemasXML::Load(worldData, worldFileName, "World"))
		worldData.SetDefault();

	// Starting Sector, Position and facing etc.
	SSharedDataRootHostToKernel::Camera().m_sector = SData::GetEntryById(worldData.m_Sector, worldData.m_StartSector, c_defaultSector);
	CGame::SetPlayerPos(worldData.m_StartPoint.m_x, worldData.m_StartPoint.m_y, worldData.m_StartPoint.m_z);
	CGame::SetPlayerFacing(worldData.m_StartFacing.m_x, worldData.m_StartFacing.m_y, worldData.m_StartFacing.m_z);
	CCamera::Get().SetAutoAjustBrightness(worldData.m_AutoAdjustBrightness);

	// portals
	m_portals.Resize(worldData.m_Portal.size());
	for (unsigned int index = 0, count = worldData.m_Portal.size(); index < count; ++index)
	{
		m_portals[index].m_sector = SData::GetEntryById(worldData.m_Sector, worldData.m_Portal[index].m_Sector, c_defaultSector);
		Copy(m_portals[index].m_xaxis, worldData.m_Portal[index].m_XAxis);
		Copy(m_portals[index].m_yaxis, worldData.m_Portal[index].m_YAxis);
		Copy(m_portals[index].m_zaxis, worldData.m_Portal[index].m_ZAxis);
		Copy(m_portals[index].m_waxis, worldData.m_Portal[index].m_WAxis);

		m_portals[index].m_setPosition = worldData.m_Portal[index].m_SetPosition;
		Copy(m_portals[index].m_position, worldData.m_Portal[index].m_Position);
	}

	// materials
	m_materials.Presize(worldData.m_Material.size() + 1);
	AddDebugMaterial();
	for (unsigned int index = 0, count = worldData.m_Material.size(); index < count; ++index)
		AddMaterial(worldData.m_Material[index]);

	// models
	m_namedModels.reserve(worldData.m_Model.size());
	for (unsigned int index = 0, count = worldData.m_Model.size(); index < count; ++index)
		AddModel(worldData.m_Model[index]);

	// sectors
	m_sectors.Resize(worldData.m_Sector.size());
	for (unsigned int sectorIndex = 0, sectorCount = worldData.m_Sector.size(); sectorIndex < sectorCount; ++sectorIndex)
		LoadSector(m_sectors[sectorIndex], worldData.m_Sector[sectorIndex], worldData.m_Material, worldData.m_Portal);

	// calculate the texture indices of our textures
	for (unsigned int index = 0, count = m_materials.Count(); index < count; ++index)
	{
		const float add = -0.5f / ((float)CDirectX::TextureManager().NumTextures());

		m_materials[index].m_diffuseTextureIndex = (m_materials[index].m_diffuseTextureIndex) / (float)CDirectX::TextureManager().NumTextures();
		m_materials[index].m_diffuseTextureIndex += add;

		m_materials[index].m_normalTextureIndex = (m_materials[index].m_normalTextureIndex) / (float)CDirectX::TextureManager().NumTextures();
		m_materials[index].m_normalTextureIndex += add;

		m_materials[index].m_emissiveTextureIndex = (m_materials[index].m_emissiveTextureIndex) / (float)CDirectX::TextureManager().NumTextures();
		m_materials[index].m_emissiveTextureIndex += add;
	}

	// combine all the textures now that they are all loaded
	CDirectX::TextureManager().FinalizeTextures();

	// handle the sector ConnectToSector fields for automatic portal generation
	for (unsigned int sectorIndex = 0, sectorCount = worldData.m_Sector.size(); sectorIndex < sectorCount; ++sectorIndex)
		HandleSectorConnectTos(sectorIndex, worldData.m_Sector);

	/*
	// handle the connect tags that connect sectors together
	for (unsigned int connectIndex = 0, connectCount = worldData.m_Connect.size(); connectIndex < connectCount; ++connectIndex)
	{
		const SData_Connect &connect = worldData.m_Connect[connectIndex];
		
		unsigned int srcSector = SData::GetEntryById(worldData.m_Sector, connect.m_SrcSector);
		unsigned int destSector = SData::GetEntryById(worldData.m_Sector, connect.m_DestSector);
		cl_float2 offset;
		Copy(offset, connect.m_Offset);
		cl_float4 portalWindow;
		Copy(portalWindow, connect.m_PortalWindow);

		// make connection from source to dest
		ConnectSectors(srcSector, connect.m_SrcSectorPlane, destSector, connect.m_DestSectorPlane, offset, portalWindow);

		// make connection from dest to source if we are supposed to
		if (connect.m_BothWays)
		{
			SSector &sector = m_sectors[srcSector];
			const SPortal &portal = m_portals[m_portals.Count() - 1];

			cl_float4 axisX,axisY,axisZ,axisW;
			GetSectorPlaneTransformationMatrix(sector, connect.m_SrcSectorPlane, axisX, axisY, axisZ, axisW);

			float3 offsetWorldSpace;
			offsetWorldSpace[0] = axisX.s[0] * offset.s[0] + axisY.s[0] * offset.s[1];
			offsetWorldSpace[1] = axisX.s[1] * offset.s[0] + axisY.s[1] * offset.s[1];
			offsetWorldSpace[2] = axisX.s[2] * offset.s[0] + axisY.s[2] * offset.s[1];

			cl_float2 newOffset;
			//newOffset.s[0] = dot(

			int ijkl = 0;

			//float3 offset;
			//offset[0] = portal.

			//offset.s[0] *= -1.0f;
			//offset.s[1] *= -1.0f;
			ConnectSectors(destSector, connect.m_DestSectorPlane, srcSector, connect.m_SrcSectorPlane, offset, portalWindow);
		}
	}
	*/

	return true;
}