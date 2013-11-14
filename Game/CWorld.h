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
		m_materials.Release();
	}

	bool Load(const char *worldFileName);

private:
	friend class CDirectX;

	CSharedArray<SPointLight>	m_pointLights;
	CSharedArray<SSphere>		m_spheres;
	CSharedArray<SAABox>		m_boxes;
	CSharedArray<SMaterial>		m_materials;
};