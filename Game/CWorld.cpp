/*==================================================================================================

CWorld.cpp

This class holds all information about the world

==================================================================================================*/

#include "Platform\CDirectx.h"

#include "CWorld.h"

#include "DataSchemas/DataSchemasXML.h"

#include "CGame.h"
#include "MatrixMath.h"
#include "CDaeModelLoader.h"

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
		Copy(plane.m_textureOffset, planeSource.m_TextureOffset);
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
		Copy(box.m_textureOffset, boxSource.m_TextureOffset);

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
		Copy(sphere.m_textureOffset, sphereSource.m_TextureOffset);

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
void CWorld::LoadSectorModels (
	SSector &sector,
	struct SData_Sector &sectorSource,
	std::vector<struct SData_Material> &materials,
	std::vector<struct SData_Portal> &portals
) {

	for (unsigned int index = 0, count = sectorSource.m_Model.size(); index < count; ++index)
	{
		SData_Model &model = sectorSource.m_Model[index];
		SData_DAEFILE modelData;
		if (!DataSchemasXML::Load(modelData, model.m_FileName.c_str(), "COLLADA"))
			continue;

		int ijkl = 0;
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
		unsigned int connectToSectorIndex = SData::GetEntryById(sectorsSource, sectorPlaneSource.m_ConnectToSector);
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
		Copy(plane.m_textureOffset, planeSource.m_TextureOffset);

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
	LoadSectorModels(sector, sectorSource, materials, portals);
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

		m_portals[index].m_setPosition = worldData.m_Portal[index].m_SetPosition;
		Copy(m_portals[index].m_position, worldData.m_Portal[index].m_Position);
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