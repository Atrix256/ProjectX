/*==================================================================================================

SSharedDataRoot.cpp

This holds all constant sized data that we send to the kernal program

==================================================================================================*/

#include "SSharedDataRoot.h"

static CSharedObject<SSharedDataRootHostToKernel> s_dataHostToKernel;
static CSharedObject<SSharedDataRootKernelToHost> s_dataKernelToHost;

CSharedObject<SSharedDataRootHostToKernel>& SSharedDataRootHostToKernel::Get()
{
	return s_dataHostToKernel;
}

CSharedObject<SSharedDataRootKernelToHost>& SSharedDataRootKernelToHost::Get()
{
	return s_dataKernelToHost;
}

SCamera &SSharedDataRootHostToKernel::Camera()
{
	return s_dataHostToKernel.GetObject().m_camera;
}

const SCamera &SSharedDataRootHostToKernel::CameraConst()
{
	return s_dataHostToKernel.GetObjectConst().m_camera;
}
