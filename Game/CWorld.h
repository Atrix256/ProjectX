/*==================================================================================================

CWorld.h

This class holds all information about the world

==================================================================================================*/

#pragma once

#include "Platform/SharedArray.h"
#include "KernelCode/Shared/SharedGeometry.h"
#include "KernelCode/Shared/SSharedDataRoot.h"
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
		m_boxes.Release();
		m_planes.Release();
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

private:
	friend class CDirectX;

	void LoadSector (
		SSector &sector,
		struct SData_Sector &sectorSource,
		std::vector<struct SData_Material> &materials,
		std::vector<struct SData_Portal> &portals
	);

	void LoadSectorPlanes (
		SSector &sector,
		struct SData_Sector &sectorSource,
		std::vector<struct SData_Material> &materials,
		std::vector<struct SData_Portal> &portals
	);

	void LoadSectorBoxes (
		SSector &sector,
		struct SData_Sector &sectorSource,
		std::vector<struct SData_Material> &materials,
		std::vector<struct SData_Portal> &portals
	);

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

	void LoadSectorModels (
		SSector &sector,
		struct SData_Sector &sectorSource,
		std::vector<struct SData_Material> &materials,
		std::vector<struct SData_Portal> &portals
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

	CSharedArray<SPointLight>	m_pointLights;
	CSharedArray<SSphere>		m_spheres;
	CSharedArray<SAABox>		m_boxes;
	CSharedArray<SPlane>		m_planes;
	CSharedArray<SSector>		m_sectors;
	CSharedArray<SMaterial>		m_materials;
	CSharedArray<SPortal>		m_portals;

	unsigned int m_nextObjectId;
};