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
		if (camera.m_pos[0] >= -sectors[index].m_halfDims[0] &&
			camera.m_pos[0] <=  sectors[index].m_halfDims[0] &&
			camera.m_pos[1] >= -sectors[index].m_halfDims[1] &&
			camera.m_pos[1] <=  sectors[index].m_halfDims[1] &&
			camera.m_pos[2] >= -sectors[index].m_halfDims[2] &&
			camera.m_pos[2] <=  sectors[index].m_halfDims[2])
		{
			camera.m_sector = index;
			break;
		}
	}	
}

float CCamera::CurrentGroundHeight () const {
	const SCamera &camera = SSharedDataRoot::CameraConst();
	const CWorld &world = CDirectX::GetWorld();
	unsigned int numSectors = 0;
	const SSector* sectors = world.GetSectors(numSectors);
	if (camera.m_sector < numSectors)
		return -sectors[camera.m_sector].m_halfDims[1];

	return 0.0f;
}
