/*==================================================================================================

CWorld.h

This class holds all information about the world

==================================================================================================*/

#pragma once

#include "Platform/SharedArray.h"
#include "KernelCode/Shared/SharedGeometry.h"
#include "KernelCode/Shared/SSharedDataRoot.h"

class CWorld
{
public:
	CWorld() { }
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

private:
	friend class CDirectX;

	CSharedArray<SPointLight>	m_pointLights;
	CSharedArray<SSphere>		m_spheres;
	CSharedArray<SAABox>		m_boxes;
	CSharedArray<SPlane>		m_planes;
	CSharedArray<SSector>		m_sectors;
	CSharedArray<SMaterial>		m_materials;
	CSharedArray<SPortal>		m_portals;
};