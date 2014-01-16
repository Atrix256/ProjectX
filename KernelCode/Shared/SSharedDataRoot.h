/*==================================================================================================

SSharedDataRoot.h

This holds all constant sized data that we send to the kernal program

==================================================================================================*/

#pragma once

#include "SCamera.h"

#ifndef OPENCL
#include "Platform/SharedObject.h"
#endif

struct SSharedDataRoot
{
	struct SCamera		m_camera;

#ifndef OPENCL
	static CSharedObject<SSharedDataRoot>& Get();
	static SCamera &Camera();
	static const SCamera &CameraConst();
#endif
};

