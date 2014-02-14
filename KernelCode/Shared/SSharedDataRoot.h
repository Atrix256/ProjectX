/*==================================================================================================

SSharedDataRoot.h

This holds all constant sized data that we send to the kernal program

==================================================================================================*/

#pragma once

#include "SCamera.h"

#ifndef OPENCL
#include "Platform/SharedObject.h"
#endif

struct SSharedDataRootHostToKernel
{
	struct SCamera		m_camera;

#ifndef OPENCL
	static CSharedObject<SSharedDataRootHostToKernel>& Get();
	static SCamera &Camera();
	static const SCamera &CameraConst();
#endif
};

struct SSharedDataRootKernelToHost
{
	unsigned int m_maxBrightness1000x;

	unsigned int m_pad1;
	unsigned int m_pad2;
	unsigned int m_pad3;

#ifndef OPENCL
	void PreRender ()
	{
		Get().GetObject().m_maxBrightness1000x = 0;
	}

	static CSharedObject<SSharedDataRootKernelToHost>& Get();
#endif
};