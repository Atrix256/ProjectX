/*==================================================================================================

SharedTypes.h

defines simple, common types for use in code shared between the cpu and gpu code

==================================================================================================*/

#pragma once

#ifndef OPENCL
	#include "Platform/float3.h"
#else
	typedef unsigned int cl_uint;
	typedef float cl_float;
	typedef float2 cl_float2;
	typedef float4 cl_float4;
	typedef unsigned char cl_uchar;
#endif