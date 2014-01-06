/*==================================================================================================

CCamera.cpp

This is the camera singleton class.  Manipulate this object to change the view of the world.

==================================================================================================*/

#include "Platform\CDirectx.h"
#include "CCamera.h"

// singleton
CCamera CCamera::s_camera;

void CCamera::OnMove ()
{
	// figure out which sector the camera is in now
	SCamera &camera = SSharedDataRoot::Camera();
	const CWorld &world = CDirectX::GetWorld();
	camera.m_sector = -1;
	unsigned int numSectors = 0;
	const SSector* sectors = world.GetSectors(numSectors);
	for (unsigned int index = 0; index < numSectors; ++index)
	{
		bool inPlanes = true;
		for (unsigned int planeIndex = 0; planeIndex < SSECTOR_NUMPLANES; ++planeIndex)
		{
			float side = camera.m_pos[0] * sectors[index].m_planes[planeIndex].m_equation.s[0]
					   + camera.m_pos[1] * sectors[index].m_planes[planeIndex].m_equation.s[1]
					   + camera.m_pos[2] * sectors[index].m_planes[planeIndex].m_equation.s[2]
					   + sectors[index].m_planes[planeIndex].m_equation.s[3];

			if (side < 0.0f)
			{
				inPlanes = false;
				break;
			}
		}

		if (inPlanes)
		{
			camera.m_sector = index;
			break;
		}
	}	
}