/*==================================================================================================

SWorld.h

This holds the constant sized information about the world

==================================================================================================*/

#pragma once

#ifndef OPENCL
	#include "Platform/float3.h"
#else
	typedef int cl_int;
#endif

struct SWorld
{
	float3 m_ambientLight;
	
	cl_int m_numLights;
	cl_int m_numSpheres;
	cl_int m_numBoxes;
	cl_int m_numPlanes;

	cl_int m_numSectors;
	cl_int m_numMaterials;
	cl_int m_pad[2];
};