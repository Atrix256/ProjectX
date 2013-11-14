/*==================================================================================================

SSharedDataRoot.cpp

This holds all constant sized data that we send to the kernal program

==================================================================================================*/

#include "SSharedDataRoot.h"

static CSharedObject<SSharedDataRoot> s_data;

CSharedObject<SSharedDataRoot>& SSharedDataRoot::Get()
{
	return s_data;
}

SCamera &SSharedDataRoot::Camera()
{
	return s_data.GetObject().m_camera;
}

const SCamera &SSharedDataRoot::CameraConst()
{
	return s_data.GetObjectConst().m_camera;
}

SWorld &SSharedDataRoot::World()
{
	return s_data.GetObject().m_world;
}

const SWorld &SSharedDataRoot::WorldConst()
{
	return s_data.GetObjectConst().m_world;
}