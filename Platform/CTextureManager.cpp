/*==================================================================================================

CTextureManager.cpp

Holds loaded textures and allows for passing them to the kernel code

==================================================================================================*/

#include "CDirectx.h"
#include "CTextureManager.h"

//-----------------------------------------------------------------------------
void CTextureManager::Init ()
{
	m_textureSize = CDirectX::Settings().m_TextureSize;
}

//-----------------------------------------------------------------------------
unsigned int CTextureManager::GetOrLoad (const char *fileName)
{
	for (unsigned int index = 0; index < m_numTextures; ++index)
	{
		if (!stricmp(fileName, m_textures[index].m_fileName.c_str()))
			return index + 1;
	}

	Assert_(m_numTextures < c_maxTextures);

	if (LoadTexture(fileName, &m_textures[m_numTextures].pTexture, m_textures[m_numTextures].clTexture))
	{
		m_textures[m_numTextures].m_fileName = fileName;
		m_numTextures++;
		return m_numTextures;
	}

	return 0;
}

//-----------------------------------------------------------------------------
void CTextureManager::FinalizeTextures ()
{
	// create the 3d texture
	cl_image_format imageFormat;
	imageFormat.image_channel_order = CL_RGBA;
	imageFormat.image_channel_data_type = CL_FLOAT;

	int ciErrNum = 0;
	m_clTexture3d = clCreateImage3D(
		CDirectX::Get().m_cxGPUContext,
		CL_MEM_READ_ONLY,
		&imageFormat,
		m_textureSize,
		m_textureSize,
		m_numTextures > 1 ? m_numTextures : 2, // depth - need at least 2 textures
		0, // row pitch
		0, // slice pitch
		NULL,
		&ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

	for (unsigned int index = 0; index < m_numTextures; ++index)
		MoveTextureToCL(index);

}

//-----------------------------------------------------------------------------
void CTextureManager::MoveTextureToCL (int index)
{
	// map the texture
	STexture2D &texture = m_textures[index];
	D3D10_TEXTURE2D_DESC desc;
	texture.pTexture->GetDesc(&desc);
	Assert_(desc.Width == desc.Height); // textures must be square
	D3D10_MAPPED_TEXTURE2D mapped2dTexture;
	HRESULT hr = texture.pTexture->Map(0, D3D10_MAP_READ, 0, &mapped2dTexture);
	Assert_(!FAILED(hr));
	
	// make the image data so we can pass it to opencl
	float *imageData = new float[m_textureSize * m_textureSize * 4];
	float *destPixel = imageData;
	for (unsigned int indexY = 0; indexY < m_textureSize; ++indexY)
	{
		float percentY = (float)indexY / (float)m_textureSize;
		for (unsigned int indexX = 0; indexX < m_textureSize; ++indexX)
		{
			float percentX = (float)indexX / (float)m_textureSize;
			SampleMappedPixelBilinear(mapped2dTexture, destPixel, percentX, percentY, desc.Width, desc.Height);
			destPixel += 4;
		}
	}

	// unmap and release the texture
	texture.pTexture->Unmap(0);
	texture.Release();

	// send the image data to opencl
	const size_t origin[3] = {0, 0, index};
	const size_t region[3] = {m_textureSize, m_textureSize, 1};

	int ciErrNum = clEnqueueWriteImage(
		CDirectX::Get().m_cqCommandQueue,
		m_clTexture3d,
		true,
		origin,
		region,
		0,
		0,
		imageData,
		0,
		0,
		0);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

	// free the image data
	delete[] imageData;
}

//-----------------------------------------------------------------------------
void CTextureManager::SampleMappedPixelBilinear(
	D3D10_MAPPED_TEXTURE2D& mapped2dTexture,
	float *destPixel,
	float percentX,
	float percentY,
	unsigned int width,
	unsigned int height)
{
	// calculate our texel coordinates
	float srcCoordX = percentX * (float)width;
	float srcCoordY = percentY * (float)height;

	// get pointers to the 4 texels we are sampling
	unsigned char *pUL = ((unsigned char *)mapped2dTexture.pData + (int)srcCoordY * mapped2dTexture.RowPitch) + (int)srcCoordX * 4;
	unsigned char *pUR = pUL + 4;
	unsigned char *pBL = pUL + mapped2dTexture.RowPitch;
	unsigned char *pBR = pBL + 4;

	// calculate the weights for each texel
	float fracx = srcCoordX - floorf(srcCoordX);
	float fracy = srcCoordY - floorf(srcCoordY);
	float wUL = (1.0f - fracx) * (1.0f - fracy);
	float wUR = fracx * (1.0f - fracy);
	float wBL = (1.0f - fracx) * fracy;
	float wBR = fracx *  fracy;

	//color = pUL * wUL + pUR * wUR + pBL * wBL + pBR * wBR
	for (unsigned int index = 0; index < 4; ++index)
	{
		destPixel[index] =
			((float)pUL[index]) / 256.0f * wUL +
			((float)pUR[index]) / 256.0f * wUR +
			((float)pBL[index]) / 256.0f * wBL +
			((float)pBR[index]) / 256.0f * wBR;
	}
}

//-----------------------------------------------------------------------------
bool CTextureManager::LoadTexture(const char *fileName, ID3D10Texture2D **d3dTexture, cl_mem & clTexture)
{
	D3DX10_IMAGE_LOAD_INFO loadInfo;
	loadInfo.Width = D3DX10_DEFAULT;  
	loadInfo.Height = D3DX10_DEFAULT;
	loadInfo.Depth = D3DX10_DEFAULT;
	loadInfo.FirstMipLevel = D3DX10_DEFAULT;
	loadInfo.MipLevels = D3DX10_DEFAULT;
	loadInfo.Usage = D3D10_USAGE_STAGING;
	loadInfo.BindFlags = 0;
	loadInfo.CpuAccessFlags = D3D10_CPU_ACCESS_READ;
	loadInfo.MiscFlags = D3DX10_DEFAULT;
	loadInfo.Filter = D3DX10_FILTER_NONE;
	loadInfo.MipFilter = D3DX10_FILTER_NONE;
	loadInfo.pSrcInfo = NULL;
	return !FAILED(D3DX10CreateTextureFromFile( CDirectX::Get().m_pd3dDevice, fileName, &loadInfo, NULL, (ID3D10Resource**)d3dTexture, NULL ));
}

