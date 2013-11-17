/*==================================================================================================

STexture2D.h

Data structure for 2D texture shared between DX and CL

==================================================================================================*/

#pragma once

// OpenCL includes
#include "oclUtils.h"
#include <CL/cl_d3d10.h>
#include <CL/cl_d3d10_ext.h>
#include <CL/cl_ext.h>

#include <string>

struct STexture2D
{
	STexture2D ()
	{
		pTexture = NULL;
		pSRView = NULL;
		clTexture = NULL;
	}

	void Release ()
	{
		if (pSRView)
		{
			pSRView->Release();
			pSRView = NULL;
		}

		if (pTexture)
		{
			pTexture->Release();
			pTexture = NULL;
		}

		if (clTexture)
		{
			clReleaseMemObject(clTexture);
			clTexture = NULL;
		}
	}

	std::string				m_fileName;
    ID3D10Texture2D			*pTexture;
    ID3D10ShaderResourceView *pSRView;
	cl_mem				clTexture;
	unsigned int		pitch;
	unsigned int		width;
	unsigned int		height;	
};
