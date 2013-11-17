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
	static const unsigned int c_maxTextures = 16;

	CTextureManager()
	{
		m_numTextures = 0;
		m_texture3d = NULL;
		m_clTexture3d = NULL;
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

		if (m_texture3d)
		{
			m_texture3d->Release();
			m_texture3d = NULL;
		}
	}

	cl_mem GetCLTexture (unsigned int index)
	{
		Assert_(index < m_numTextures);
		return m_textures[index].clTexture;
	}

	cl_mem GetCLTexture3d ()
	{
		return m_clTexture3d;
	}

	unsigned int NumTextures () { return m_numTextures; }

	unsigned int GetOrLoad (const char *fileName);

	// this finalizes the textures for use in openck and frees the directx resources
	void FinalizeTextures ();

private:
	void MoveTextureToCL (int index);

	void SampleMappedPixelBilinear(
		D3D10_MAPPED_TEXTURE2D& mapped2dTexture,
		float *destPixel,
		float percentX,
		float percentY,
		unsigned int width,
		unsigned int height);

	bool LoadTexture(const char *fileName, ID3D10Texture2D **d3dTexture, cl_mem & clTexture);

	STexture2D		m_textures[c_maxTextures];
	unsigned int	m_numTextures;

	ID3D10Texture3D *m_texture3d;
	cl_mem			m_clTexture3d;

	static const int c_textureSize = 512;  // the width and height of textures used by opencl
};