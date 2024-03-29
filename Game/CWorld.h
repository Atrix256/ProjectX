/*==================================================================================================

CWorld.h

This class holds all information about the world

==================================================================================================*/

#pragma once

#include "Platform/SharedArray.h"
#include "KernelCode/Shared/SharedGeometry.h"
#include "KernelCode/Shared/SSharedDataRoot.h"
#include "DataSchemas/DataSchemasStructs.h"
#include <vector>

class CWorld
{
public:
	CWorld() { m_nextObjectId = 1; }
	~CWorld() { Release(); }

	void Release ()
	{
		m_pointLights.Release();
		m_spheres.Release();
		m_modelTriangles.Release();
		m_modelObjects.Release();
		m_modelInstances.Release();
		m_sectors.Release();
		m_materials.Release();
		m_portals.Release();
	}

	bool Load(const char *worldFileName);

	const SSector* GetSectors(unsigned int& numSectors) const
	{
		numSectors = m_sectors.Count();
		return m_sectors.DataConst();
	}

	const SPortal* GetPortals(unsigned int& numPortals) const
	{
		numPortals = m_portals.Count();
		return m_portals.DataConst();
	}

	unsigned int GetSectorIDByName (const char *sector) const;

private:
	friend class CDirectX;

	void LoadSector (
		SSector &sector,
		struct SData_Sector &sectorSource,
		std::vector<struct SData_Material> &materials,
		std::vector<struct SData_Portal> &portals
	);

	// temp - until models are working more fully and the other (useless) primitives go away
	void AddTriangle (
		const struct SData_Vec3 &sa,
		const struct SData_Vec3 &sb,
		const struct SData_Vec3 &sc,
		const struct SData_Vec2 &ta,
		const struct SData_Vec2 &tb,
		const struct SData_Vec2 &tc,
		const struct SData_Vec3 &normal,
		const struct SData_Vec3 &tangent,
		const struct SData_Vec3 &bitangent
	);

	unsigned int AddMaterial (const struct SData_Material &materialSource, const char *path ="./");
	void AddDebugMaterial ();

	void AddModel (const struct SData_Model &modelSource);

	void SortTrianglesByHalfSpace (SModelObject &object);

	void LoadSectorSpheres (
		SSector &sector,
		struct SData_Sector &sectorSource,
		std::vector<struct SData_Material> &materials,
		std::vector<struct SData_Portal> &portals
	);

	void LoadSectorPointLights (
		SSector &sector,
		struct SData_Sector &sectorSource,
		std::vector<struct SData_Material> &materials,
		std::vector<struct SData_Portal> &portals
	);

	void LoadSectorModelInstances (
		SSector &sector,
		struct SData_Sector &sectorSource,
		std::vector<struct SData_Material> &materials,
		std::vector<struct SData_Portal> &portals
	);

	void CalculateModelFarthestPoint (
		const struct SData_XMDFILE &modelData,
		float3 &point
	);

	void HandleSectorConnectTos (
		unsigned int sectorIndex,
		const std::vector<struct SData_Sector> &sectorsSource
	);

	void ConnectSectors (
		unsigned int sectorIndex,
		unsigned int planeIndex,
		unsigned int destSectorIndex,
		unsigned int destPlaneIndex,
		const cl_float2 &offset,
		const cl_float4 &portalWindow,
		bool setPosition,
		const float3 &position
	);

	void GetSectorPlaneTransformationMatrix (
		const SSector &sector,
		unsigned int planeIndex,
		cl_float4 &xAxis,
		cl_float4 &yAxis,
		cl_float4 &zAxis,
		cl_float4 &wAxis
	);

	void GetSectorPlaneInverseTransformationMatrix (
		const SSector &sector,
		unsigned int planeIndex,
		cl_float4 &xAxis,
		cl_float4 &yAxis,
		cl_float4 &zAxis,
		cl_float4 &wAxis
	);

	void GetSectorPlaneDimenions (
		const SSector &sector,
		unsigned int planeIndex,
		float &minX,
		float &minY,
		float &maxX,
		float &maxY
	);

	// used to store the models loaded in the root section of a level
	struct SNamedModel
	{
		std::string	m_id;
		cl_uint		m_startObjectIndex;
		cl_uint		m_stopObjectIndex;
		float3		m_farthestPointFromOrigin;
	};

	CSharedArray<SPointLight>		m_pointLights;
	CSharedArray<SSphere>			m_spheres;
	CSharedArray<SModelTriangle>	m_modelTriangles;	// a face
	CSharedArray<SModelObject>		m_modelObjects;		// objects are a collection of triangles
	CSharedArray<SModelInstance>	m_modelInstances;	// a list of objects, along with a bounding sphere and a transform object
	CSharedArray<SSector>			m_sectors;
	CSharedArray<SMaterial>			m_materials;
	CSharedArray<SPortal>			m_portals;

	// the models specified in the level file
	std::vector<SNamedModel>		m_namedModels;

	// the currently loaded world data
	SData_World m_worldData;

	// next OpenCL object ID
	unsigned int m_nextObjectId;
};