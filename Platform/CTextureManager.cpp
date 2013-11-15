/*==================================================================================================

CTextureManager.cpp

Holds loaded textures and allows for passing them to the kernel code

==================================================================================================*/

#include "CDirectx.h"
#include "CTextureManager.h"

//-----------------------------------------------------------------------------
unsigned int CTextureManager::GetOrLoad (const char *fileName)
{
	for (unsigned int index = 0; index < m_numTextures; ++index)
	{
		if (!stricmp(fileName, m_textures[index].m_fileName.c_str()))
			return index + 1;
	}

	Assert_(m_numTextures < c_maxTextures);

	if (CDirectX::Get().LoadTexture(fileName, &m_textures[m_numTextures].pTexture, m_textures[m_numTextures].clTexture))
	{
		m_textures[m_numTextures].m_fileName = fileName;
		m_numTextures++;
		return m_numTextures;
	}

	return 0;
}