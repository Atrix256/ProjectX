/*==================================================================================================

CCamera.h

This is the camera singleton class.  Manipulate this object to change the view of the world.

==================================================================================================*/

#pragma once

#include "Platform/float3.h"
#include "KernelCode/Shared/SSharedDataRoot.h"

class CCamera
{
public:
	CCamera ()
	{
		SCamera &cameraShared = SSharedDataRootHostToKernel::Camera();

		cameraShared.m_sector = 0;

		cameraShared.m_pos[0] = 0.0f;
		cameraShared.m_pos[1] = 0.0f;
		cameraShared.m_pos[2] = 0.0f;

		cameraShared.m_fwd[0] = 0.0f;
		cameraShared.m_fwd[1] = 0.0f;
		cameraShared.m_fwd[2] = 1.0f;

		cameraShared.m_up[0] = 0.0f;
		cameraShared.m_up[1] = 1.0f;
		cameraShared.m_up[2] = 0.0f;

		cameraShared.m_left[0] = -1.0f;
		cameraShared.m_left[1] =  0.0f;
		cameraShared.m_left[2] =  0.0f;

		cameraShared.m_frameCount = 0;

		// width
		cameraShared.m_viewWidthHeightDistance[0] = 6.0f;

		// height (will be calculated later)
		cameraShared.m_viewWidthHeightDistance[1] = 1.0f;

		// distance
		cameraShared.m_viewWidthHeightDistance[2] = 6.0f;

		cameraShared.m_brightnessMultiplier = 1.0f;
	}

	// singleton access
	static CCamera& Get () { return s_camera; }

	void SetBearings (float3 &pos, float3 &fwd, float3 &left, float3 &up, cl_uint &sector)
	{
		SCamera &camera = SSharedDataRootHostToKernel::Camera();
		camera.m_pos = pos;
		camera.m_fwd = fwd;
		camera.m_left = left;
		camera.m_up = up;
		camera.m_sector = sector;
		//float3 delta = {0,0,0};
		//AttemptMove(delta);
	}

	void AttemptMove (const float3 &delta);

	float CurrentGroundHeight () const;


private:
	// singleton
	static CCamera s_camera;
};