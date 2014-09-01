/*==================================================================================================

CDirectX.h

DirectX and OpenCL code lives here

==================================================================================================*/

#pragma once

// TODO: this is temporary
#pragma warning ( disable : 4005 )

// Direct3D9 includes
#include <d3d9.h>
#include <d3dx9.h>

// Direct3D10 includes
#include <dxgi.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include <d3dx10.h>

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#include <string>

// OpenCL includes
#include "oclUtils.h"
#include <CL/cl_d3d10.h>
#include <CL/cl_d3d10_ext.h>
#include <CL/cl_ext.h>

#include "Game/CWorld.h"
#include "STexture2D.h"
#include "CTextureManager.h"
#include "DataSchemas/DataSchemasXML.h"

class CDirectX
{
public:
	~CDirectX ();

	bool Init ();

	void DrawScene (float elapsed);

	static CDirectX& Get () { return s_singleton; } 

	HRESULT InitTextures ();
	HRESULT InitCL ();

	HWND GetHWND () const { return m_hWnd; }

	void LoadGraphicsSettings ();

	void SetWorld (const char *world) {m_worldFileName = world;}

	void TakeScreenshot (const char *fileName);
	void TakeScreenshot ();

	void RequestScreenshot ();

	void ToggleRecording ();

	bool IsRecording () const { return m_recording; }

	static const int c_recordingFPS = 30;

	static CTextureManager &TextureManager() { return Get().m_textureManager; };

	static const CWorld& GetWorld () { return Get().m_world; }

	static const SData_GfxSettings& Settings () { return Get().m_graphicsSettings; }

private:
	friend class CTextureManager;

	CDirectX ();
	HRESULT InitD3D10 ();

	void RunCL (float elapsed);
	void RunKernels (float elapsed);

	void AcquireTexturesForOpenCL ();
	void ReleaseTexturesFromOpenCL ();

	HRESULT CreateKernelProgram (
		const char *clName,
		const char *clPtx,
		const char *kernelEntryPoint,
		cl_program			&cpProgram,
		cl_kernel			&ckKernel
	);

private:
	static CDirectX			s_singleton;

	ID3D10Device*           m_pd3dDevice;
	IDXGISwapChain*         m_pSwapChain;
	ID3D10RenderTargetView* m_pSwapChainRTV;
	ID3D10RasterizerState*  m_pRasterState;
	HWND					m_hWnd;

	ID3D10InputLayout*      m_pInputLayout;
	ID3D10Effect*           m_pSimpleEffect;
	ID3D10EffectTechnique*  m_pSimpleTechnique;
	ID3D10EffectShaderResourceVariable* m_pTexture2D;

	STexture2D				m_texture_2d;

	clGetDeviceIDsFromD3D10KHR_fn		m_clGetDeviceIDsFromD3D10KHR;
	clCreateFromD3D10Texture2DKHR_fn		m_clCreateFromD3D10Texture2DKHR;
	clEnqueueAcquireD3D10ObjectsKHR_fn	m_clEnqueueAcquireD3D10ObjectsKHR;
	clEnqueueReleaseD3D10ObjectsKHR_fn	m_clEnqueueReleaseD3D10ObjectsKHR;

	cl_context			m_cxGPUContext;
	cl_command_queue	m_cqCommandQueue;
	//cl_device_id		m_device;
	cl_program			m_cpProgram_tex2d;
	cl_kernel			m_ckKernel_tex2d;
	size_t				m_szGlobalWorkSize[2];
	size_t				m_szLocalWorkSize[2];

	SData_GfxSettings	m_graphicsSettings;

	CWorld				m_world;
	std::string			m_worldFileName;

	CTextureManager		m_textureManager;

	HWND			      g_hWnd;
	D3DDISPLAYMODE        g_d3ddm;    
	D3DPRESENT_PARAMETERS g_d3dpp;

	WNDCLASSEX m_wc;

	unsigned int			m_width;
	unsigned int			m_height;
	bool					m_recording;
	unsigned int			m_recordingFrameNumber;
	bool					m_wantsScreenshot;
};