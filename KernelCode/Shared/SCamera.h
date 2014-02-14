/*==================================================================================================

SCamera.h

This holds the constant sized information about the camera

==================================================================================================*/

#pragma once

#include "SharedTypes.h"

struct SCamera
{
	float3 m_pos;
	float3 m_fwd;
	float3 m_up;
	float3 m_left;
	float3 m_viewWidthHeightDistance;
	
	cl_uint m_sector;     // which portal sector the camera is in
	cl_uint m_frameCount; // used for interlaced rendering
	float   m_brightnessMultiplier;
	unsigned int m_HDRBrightnessSamplingInterval;
};