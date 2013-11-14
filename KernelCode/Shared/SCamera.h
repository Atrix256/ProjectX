/*==================================================================================================

SCamera.h

This holds the constant sized information about the camera

==================================================================================================*/

#pragma once

#ifndef OPENCL
#include "Platform/float3.h"
#endif

struct SCamera
{
	float3 m_pos;
	float3 m_fwd;
	float3 m_up;
	float3 m_left;
	float3 m_viewWidthHeightDistance;
};