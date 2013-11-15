/*==================================================================================================

CTextureManager.h

Holds loaded textures and allows for passing them to the kernel code

Note that texture indexes start at 1 and texture 0 is reserved for "no texture"

==================================================================================================*/

#pragma once

#include "STexture2D.h"
#include "Platform/Assert.h"

class CTextureManager
{
public:
	static const unsigned int c_maxTextures = 1;

	CTextureManager()
	{
		m_numTextures = 0;
	}

	~CTextureManager()
	{
		Release();
	}

	void Release ()
	{
		for (unsigned int index = 0; index < c_maxTextures; ++index)
			m_textures[index].Release();

		m_numTextures = 0;
	}

	cl_mem GetCLTexture (unsigned int index)
	{
		Assert_(index < m_numTextures);
		return m_textures[index].clTexture;
	}

	unsigned int NumTextures () { return m_numTextures; }

	unsigned int GetOrLoad (const char *fileName);

private:
	STexture2D		m_textures[c_maxTextures];
	unsigned int	m_numTextures;
};