/*==================================================================================================

SSharedDataRoot.h

This holds all constant sized data that we send to the kernal program

==================================================================================================*/

#pragma once

#include "SCamera.h"
#include "SWorld.h"

#ifndef OPENCL
#include "Platform/SharedObject.h"
#endif

struct SSharedDataRoot
{
	struct SCamera	m_camera;
	struct SWorld	m_world;

#ifndef OPENCL
	static CSharedObject<SSharedDataRoot>& Get();
	static SCamera &Camera();
	static const SCamera &CameraConst();
	static SWorld &World();
	static const SWorld &WorldConst();
#endif
};

