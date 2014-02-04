/*==================================================================================================

CDirectX.cpp

DirectX and OpenCL code lives here

==================================================================================================*/

#include "CDirectx.h"
#include "Game/CCamera.h"
#include "Game/CGame.h"
#include "Game/CInput.h"
#include <direct.h>

#include <vector>
#include <string>

#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "dxgi.lib")

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define INITPFN(x) \
	m_ ## x = (x ## _fn)clGetExtensionFunctionAddress(#x);\
	if(!m_ ## x) { printf("failed getting " #x); return -1; }

CDirectX CDirectX::s_singleton;

// Round Up Division function
size_t shrRoundUp(int group_size, int global_size) 
{
    int r = global_size % group_size;
    if(r == 0) 
    {
        return global_size;
    } else 
    {
        return global_size + group_size - r;
    }
}

static const char g_simpleEffectSrc[] =
    "Texture2D g_Texture2D; \n" \
    "\n" \
    "SamplerState samLinear{ \n" \
    "    Filter = MIN_MAG_LINEAR_MIP_POINT; \n" \
    "};\n" \
    "\n" \
    "struct Fragment{ \n" \
    "    float4 Pos : SV_POSITION;\n" \
    "    float2 Tex : TEXCOORD0; };\n" \
    "\n" \
    "Fragment VS( uint id : SV_VertexID )\n" \
    "{\n" \
    "  Fragment Output;\n" \
    "  Output.Tex = float2((id << 1) & 2, id & 2);\n" \
    "  Output.Pos = float4(Output.Tex * float2(2,-2) + float2(-1,1), 0, 1);\n" \
    "  return Output;\n" \
    "}\n" \
    "\n" \
    "float4 PS( Fragment f ) : SV_Target\n" \
    "{\n" \
    "    return g_Texture2D.Sample( samLinear, f.Tex.xy ); \n" \
    "}\n" \
    "\n" \
    "technique10 Render\n" \
    "{\n" \
    "    pass P0\n" \
    "    {\n" \
    "        SetVertexShader( CompileShader( vs_4_0, VS() ) );\n" \
    "        SetGeometryShader( NULL );\n" \
    "        SetPixelShader( CompileShader( ps_4_0, PS() ) );\n" \
    "    }\n" \
    "}\n" \
    "\n";

CDirectX::CDirectX ()
	: m_pd3dDevice(NULL)
	, m_pSwapChain(NULL)
	, m_pSwapChainRTV(NULL)
	, m_pRasterState(NULL)
	, m_hWnd(NULL)
	, m_width(0)
	, m_height(0)
	, m_pInputLayout(NULL)
	, m_pSimpleEffect(NULL)
	, m_pSimpleTechnique(NULL)
	, m_pTexture2D(NULL)
	, m_clGetDeviceIDsFromD3D10KHR(NULL)
	, m_clCreateFromD3D10Texture2DKHR(NULL)
	, m_clEnqueueAcquireD3D10ObjectsKHR(NULL)
	, m_clEnqueueReleaseD3D10ObjectsKHR(NULL)
	, m_recording(false)
	, m_recordingFrameNumber(0)
{
}

CDirectX::~CDirectX ()
{
	if (m_pd3dDevice)
		m_pd3dDevice->Release();

    if (m_pInputLayout)
		m_pInputLayout->Release();

    if (m_pSimpleEffect)
		m_pSimpleEffect->Release();

    if (m_pSwapChainRTV)
		m_pSwapChainRTV->Release();

    if (m_pSwapChain)
		m_pSwapChain->Release();

	m_texture_2d.Release();

	m_textureManager.Release();

	m_world.Release();

	if(m_ckKernel_tex2d)
		clReleaseKernel(m_ckKernel_tex2d); 

    if(m_cpProgram_tex2d)
		clReleaseProgram(m_cpProgram_tex2d);

    if(m_cqCommandQueue)
		clReleaseCommandQueue(m_cqCommandQueue);

    if(m_cxGPUContext)
		clReleaseContext(m_cxGPUContext);

	// Unregister windows class
	UnregisterClass( m_wc.lpszClassName, m_wc.hInstance );
}

//-----------------------------------------------------------------------------
 bool CDirectX::Init (unsigned int width, unsigned int height)
{
	m_textureManager.Init();

	m_width = width;
	m_height = height;

	//
	// create window
	//
    // Register the window class
    WNDCLASSEX twc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "FPS", NULL };
	m_wc = twc;
    RegisterClassEx( &m_wc );

	int xBorder = ::GetSystemMetrics(SM_CXSIZEFRAME);
	int yMenu = ::GetSystemMetrics(SM_CYMENU);
	int yBorder = ::GetSystemMetrics(SM_CYSIZEFRAME);

    // Create the application's window (padding by window border for uniform BB sizes across OSs)
    m_hWnd = CreateWindow( m_wc.lpszClassName, "FPS",
                              WS_OVERLAPPEDWINDOW, 0, 0, m_width + 2*xBorder, m_height+ 2*yBorder+yMenu,
                              NULL, NULL, m_wc.hInstance, NULL );

    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);

	HRESULT hr = InitD3D10();
	// let's give-up if D3D failed. But we will write "succeed"
	if(FAILED(hr))
	{
		// Unregister windows class
		UnregisterClass( m_wc.lpszClassName, m_wc.hInstance );
        return false;
	}

	if(FAILED(InitCL()))
		return false;

	if(FAILED(InitTextures()))
		return false;

	// set the viewing height, based on aspect ratio of texture
	SCamera &cameraShared = SSharedDataRoot::Camera();
	cameraShared.m_viewWidthHeightDistance[1] = cameraShared.m_viewWidthHeightDistance[0] * (float)m_texture_2d.height / (float)m_texture_2d.width;

	return true;
}

 //-----------------------------------------------------------------------------
HRESULT CDirectX::InitCL()
{
    cl_platform_id	cpPlatform;

    //Get the platform
    cl_int ciErrNum = oclGetPlatformID(&cpPlatform);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

    //
	// Initialize extension functions for D3D10
	//
	INITPFN(clGetDeviceIDsFromD3D10KHR);
	INITPFN(clCreateFromD3D10Texture2DKHR);
	INITPFN(clEnqueueAcquireD3D10ObjectsKHR);
	INITPFN(clEnqueueReleaseD3D10ObjectsKHR);

	// Query the OpenCL device that would be good for the current D3D device
	// We need to take the one that is on the same Gfx card.
	
	// Get the device ids for the adapter 
    cl_device_id cdDevice; 
    cl_uint num_devices = 0;

    ciErrNum = m_clGetDeviceIDsFromD3D10KHR(
        cpPlatform,
		CL_D3D10_DEVICE_KHR,
        m_pd3dDevice,
		CL_PREFERRED_DEVICES_FOR_D3D10_KHR,
        1,
        &cdDevice,
        &num_devices);

	if (ciErrNum == -1) {
		printf("No OpenCL device available that supports D3D10, exiting...\n");
		return -1;
	} else {
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);
	}

	cl_context_properties props[] = 
    {
        CL_CONTEXT_D3D10_DEVICE_KHR, (cl_context_properties)m_pd3dDevice, 
        CL_CONTEXT_PLATFORM, (cl_context_properties)cpPlatform, 
        0
    };
    m_cxGPUContext = clCreateContext(props, 1, &cdDevice, NULL, NULL, &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

	// Log device used 
	printf("Device: ");
    oclPrintDevName(cdDevice);
    printf("\n");

    // create a command-queue
    m_cqCommandQueue = clCreateCommandQueue(m_cxGPUContext, cdDevice, 0, &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

	CreateKernelProgram("./KernelCode/clrt.cl", "clrt.ptx", "clrt", m_cpProgram_tex2d, m_ckKernel_tex2d);

	return S_OK;
}

//-----------------------------------------------------------------------------
void CDirectX::LoadGraphicsSettings ()
{
	//TODO: if graphics settings file doesn't exist, save it out so there is one!
	//NOTE: can't just check for failure of load, since we don't want to stomp whatever changes the person is making
	//      just because of a typo
	DataSchemasXML::Load(m_graphicsSettings, "./data/gfxsettings.xml", "GfxSettings");
}

//-----------------------------------------------------------------------------
void CDirectX::TakeScreenshot (const char *fileName)
{
	// backbufferSurfDesc is saved at OnResizedSwapChain call
	ID3D10Resource *backbufferRes;
	m_pSwapChainRTV->GetResource(&backbufferRes);
	
	D3D10_TEXTURE2D_DESC texDesc;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Height = m_height;
	texDesc.Width = m_width;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D10_USAGE_STAGING;

	ID3D10Texture2D *texture;
	HRESULT hr;
	hr = m_pd3dDevice->CreateTexture2D(&texDesc, 0, &texture);
	m_pd3dDevice->CopyResource(texture, backbufferRes);

	hr = D3DX10SaveTextureToFile(texture, D3DX10_IFF_BMP, fileName);
	texture->Release();
}

//-----------------------------------------------------------------------------
void CDirectX::ToggleRecording ()
{
	m_recording = !m_recording;

	// if we are starting recording, make sure the directory for frames exists
	// and set the frame number to zero;
	if (m_recording)
	{
		_mkdir("RecordedVideo");
		_chdir("RecordedVideo");
		_mkdir("Frames");
		_chdir("Frames");
		system("del *.bmp");
		_chdir("../..");
		m_recordingFrameNumber = 0;
		return;
	}

	// if we are done recording, we need to make a video out of our frames
	//find what the next output filename we can use is
	FILE *File = NULL;
	bool done = false;
	char szVideoFilename[256];
	int nextVideoIndex = 1;
	do 
	{
		sprintf(szVideoFilename,"RecordedVideo/Video%i.mp4",nextVideoIndex);
		File = fopen(szVideoFilename,"rb");

		if(File)
		{
			fclose(File);
			nextVideoIndex++;
		}
		else
		{
			done = true;
		}
	} 
	while(!done);

	//encode our video!
	char szBuffer[256];
	sprintf(szBuffer,"RecordedVideo\\ffmpeg -r %i -b:v 1800 -i RecordedVideo/Frames/frame%%d.bmp %s",c_recordingFPS,szVideoFilename);
	system(szBuffer);
}

//-----------------------------------------------------------------------------
void CDirectX::DrawScene (float elapsed)
{
	RunCL(elapsed);

    //
    // draw the 2d texture
    //
    m_pSimpleTechnique->GetPassByIndex(0)->Apply(0);
    m_pd3dDevice->Draw( 3, 0 );

    // Present the backbuffer contents to the display
	if (m_recording)
		m_pSwapChain->Present( 1, 0);
	else
		m_pSwapChain->Present( 0, 0);

	// if we are recording, take a screenshot
	if (m_recording)
	{
		char fileName[256];
		sprintf(fileName,"RecordedVideo/Frames/frame%i.bmp", m_recordingFrameNumber);
		m_recordingFrameNumber++;
		TakeScreenshot(fileName);
	}
}

//-----------------------------------------------------------------------------
HRESULT CDirectX::InitD3D10 () 
{
    HRESULT hr = S_OK;
	bool noD3DAvailable = false;

    // Select our adapter
    IDXGIAdapter* pCLCapableAdapter = NULL;
    {
        // iterate through the candidate adapters
        IDXGIFactory *pFactory;
		hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));
        if(FAILED(hr))
		{
			noD3DAvailable = true;
			return hr;
		}

        for (UINT adapter = 0; !pCLCapableAdapter; ++adapter)
        {
            // get a candidate DXGI adapter
            IDXGIAdapter* pAdapter = NULL;
            hr = pFactory->EnumAdapters(adapter, &pAdapter);
            if (FAILED(hr))
            {
                break;
            }
			// TODO: check here if the adapter is ok for CL
            {
                // if so, mark it as the one against which to create our d3d10 device
                pCLCapableAdapter = pAdapter;
				break;
            }
            pAdapter->Release();
        }
        pFactory->Release();
    }
    if(!pCLCapableAdapter)
        if(FAILED(hr))
		{
			noD3DAvailable = true;
			return E_FAIL;
		}

    // Set up the structure used to create the device and swapchain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof(sd) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = m_width;
    sd.BufferDesc.Height = m_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    // Create device and swapchain
    hr = D3D10CreateDeviceAndSwapChain( 
        pCLCapableAdapter, 
        D3D10_DRIVER_TYPE_HARDWARE, 
        NULL,
        0,
        D3D10_SDK_VERSION, 
        &sd, 
        &m_pSwapChain, 
        &m_pd3dDevice);
    if(FAILED(hr))
	{
		noD3DAvailable = true;
		return hr;
	}
    pCLCapableAdapter->Release();
	pCLCapableAdapter = NULL;

    // Create a render target view of the swapchain
    ID3D10Texture2D* pBuffer;
    hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), (LPVOID*)&pBuffer);
    if(FAILED(hr))
		return hr;

    hr = m_pd3dDevice->CreateRenderTargetView(pBuffer, NULL, &m_pSwapChainRTV);
    pBuffer->Release();
    if(FAILED(hr))
		return hr;

    m_pd3dDevice->OMSetRenderTargets(1, &m_pSwapChainRTV, NULL);

    // Setup the viewport
    D3D10_VIEWPORT vp;
    vp.Width = m_width;
    vp.Height = m_height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pd3dDevice->RSSetViewports( 1, &vp );


    // Setup the effect
    {
        ID3D10Blob* pCompiledEffect;
        ID3D10Blob* pErrors = NULL;
        hr = D3D10CompileEffectFromMemory(
            (void*)g_simpleEffectSrc,
            sizeof(g_simpleEffectSrc),
            NULL,
            NULL, // pDefines
            NULL, // pIncludes
            0, // HLSL flags
            0, // FXFlags
            &pCompiledEffect,
            &pErrors);

        if( pErrors ) 
        {
            LPVOID l_pError = NULL;
            l_pError = pErrors->GetBufferPointer(); // then cast to a char* to see it in the locals window 
            printf("Compilation error: \n %s", (char*) l_pError);
        }
		if(FAILED(hr))
			return hr;
        
        hr = D3D10CreateEffectFromMemory(
            pCompiledEffect->GetBufferPointer(),
            pCompiledEffect->GetBufferSize(),
            0, // FXFlags
            m_pd3dDevice,
            NULL,
            &m_pSimpleEffect);
        pCompiledEffect->Release();
            
        m_pSimpleTechnique = m_pSimpleEffect->GetTechniqueByName( "Render" );

		m_pTexture2D = m_pSimpleEffect->GetVariableByName("g_Texture2D")->AsShaderResource();

        // Setup  no Input Layout
        m_pd3dDevice->IASetInputLayout(0);
        m_pd3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
    }

    D3D10_RASTERIZER_DESC rasterizerState;
    rasterizerState.FillMode = D3D10_FILL_SOLID;
    rasterizerState.CullMode = D3D10_CULL_NONE;
    rasterizerState.FrontCounterClockwise = false;
    rasterizerState.DepthBias = false;
    rasterizerState.DepthBiasClamp = 0;
    rasterizerState.SlopeScaledDepthBias = 0;
    rasterizerState.DepthClipEnable = false;
    rasterizerState.ScissorEnable = false;
    rasterizerState.MultisampleEnable = false;
    rasterizerState.AntialiasedLineEnable = false;
    m_pd3dDevice->CreateRasterizerState( &rasterizerState, &m_pRasterState );
    m_pd3dDevice->RSSetState( m_pRasterState );

    return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CDirectX::InitTextures ()
{
    //
    // create the D3D resources we'll be using
    //
    // 2D texture
    {
        m_texture_2d.width  = m_width;
        m_texture_2d.pitch  = m_texture_2d.width; // for now, let's set pitch == to width
        m_texture_2d.height = m_height;

        D3D10_TEXTURE2D_DESC desc;
        ZeroMemory( &desc, sizeof(D3D10_TEXTURE2D_DESC) );
        desc.Width = m_texture_2d.width;
        desc.Height = m_texture_2d.height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D10_USAGE_DEFAULT;
        desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
        if (FAILED(m_pd3dDevice->CreateTexture2D( &desc, NULL, &m_texture_2d.pTexture)))
            return E_FAIL;

        if (FAILED(m_pd3dDevice->CreateShaderResourceView(m_texture_2d.pTexture, NULL, &m_texture_2d.pSRView)) )
            return E_FAIL;

        m_pTexture2D->SetResource( m_texture_2d.pSRView );

		// Create the OpenCL part
		cl_int ciErrNum;
		m_texture_2d.clTexture = m_clCreateFromD3D10Texture2DKHR(
			m_cxGPUContext,
			0,
			m_texture_2d.pTexture,
			0,
			&ciErrNum);

		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);
	}

	m_world.Load(m_worldFileName.c_str());

	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CDirectX::CreateKernelProgram(
	const char *clName,
	const char *clPtx,
	const char *kernelEntryPoint,
	cl_program			&cpProgram,
	cl_kernel			&ckKernel )
{
	cl_int ciErrNum;

    // Program Setup
    size_t program_length;
    //const char* source_path = shrFindFilePath(clName, exepath);
	const char* source_path = clName;
    char *source = oclLoadProgSource(source_path, "", &program_length);
	oclCheckErrorEX(source != NULL, true, NULL);

    // create the program
    cpProgram = clCreateProgramWithSource(m_cxGPUContext, 1,(const char **) &source, &program_length, &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);
    free(source);

	// make our build options
	std::string buildOptions;
	buildOptions = "-cl-fast-relaxed-math -I ./KernelCode/ -D OPENCL=1";
	buildOptions.append(" -D SETTINGS_INTERLACED=");
	buildOptions.append(m_graphicsSettings.m_InterlaceMode ? "1" : "0");
	buildOptions.append(" -D SETTINGS_NORMALMAP=");
	buildOptions.append(m_graphicsSettings.m_NormalMapping ? "1" : "0");
	buildOptions.append(" -D SETTINGS_SHADOWS=");
	buildOptions.append(m_graphicsSettings.m_Shadows ? "1" : "0");
	buildOptions.append(" -D SETTINGS_HIQLIGHTS=");
	buildOptions.append(m_graphicsSettings.m_HighQualityLights ? "1" : "0");
	buildOptions.append(" -D SETTINGS_REDBLUE3D=");
	buildOptions.append(m_graphicsSettings.m_RedBlue3D ? "1" : "0");

    // build the program
	ciErrNum = clBuildProgram(cpProgram, 0, NULL, buildOptions.c_str(), NULL, NULL);
    if (ciErrNum != CL_SUCCESS)
    {
        // write out standard error, Build Log and PTX, then cleanup and exit
		printf(" error in clBuildProgram: %i", ciErrNum);
        oclLogBuildInfo(cpProgram, oclGetFirstDev(m_cxGPUContext));
        oclLogPtx(cpProgram, oclGetFirstDev(m_cxGPUContext), clPtx);
        return -1;
    }

    // create the kernel
    ckKernel = clCreateKernel(cpProgram, kernelEntryPoint, &ciErrNum);
    if (!ckKernel)
        return -1;

    // set the args values
	return ciErrNum ? E_FAIL : S_OK;
}

//-----------------------------------------------------------------------------
void CDirectX::AcquireTexturesForOpenCL()
{
	cl_event event;
	cl_mem memToAcquire[1];
	memToAcquire[0] = m_texture_2d.clTexture;

	// do the acquire
    cl_int ciErrNum = m_clEnqueueAcquireD3D10ObjectsKHR(
        m_cqCommandQueue,
        1,
        memToAcquire,
        0,
        NULL,
        &event);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

    // make sure the event type is correct
    cl_uint eventType = 0;
    ciErrNum = clGetEventInfo(
        event,
        CL_EVENT_COMMAND_TYPE,
        sizeof(eventType),
        &eventType,
        NULL);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);
    if(eventType != CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR)
	{
		printf("event type is not CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR !\n");
	}
    ciErrNum = clReleaseEvent(event);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);
}

//-----------------------------------------------------------------------------
void CDirectX::ReleaseTexturesFromOpenCL()
{
	cl_event event;
	cl_mem memToAcquire[1];
	memToAcquire[0] = m_texture_2d.clTexture;

    // do the acquire
    cl_int ciErrNum = m_clEnqueueReleaseD3D10ObjectsKHR(
        m_cqCommandQueue,
		1,
        memToAcquire,
        0,
        NULL,
        &event);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

    // make sure the event type is correct
    cl_uint eventType = 0;
    ciErrNum = clGetEventInfo(
        event,
        CL_EVENT_COMMAND_TYPE,
        sizeof(eventType),
        &eventType,
        NULL);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);
    if(eventType != CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR)
	{
		printf("event type is not CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR !\n");
	}
    ciErrNum = clReleaseEvent(event);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);
}

//-----------------------------------------------------------------------------
void CDirectX::RunCL(float elapsed)
{
	//
	// map the resources we've registered so we can access them in cl
	// - it is most efficient to map and unmap all resources in a single call,
	//   and to have the map/unmap calls be the boundary between using the GPU
	//   for Direct3D and cl
	//

	//
	// Transfer ownership from D3D to OpenCL
	//
	AcquireTexturesForOpenCL();
    //
    // run kernels which will populate the contents of those textures
    //
    RunKernels(elapsed);
    //
    // give back the ownership to D3D
    //
	ReleaseTexturesFromOpenCL();
}

//-----------------------------------------------------------------------------
void CDirectX::RunKernels(float elapsed)
{
	// ----------------------------------------------------------------
    // render the scene
    {
		// toggle the odd / even field
		SCamera& camera = SSharedDataRoot::Camera();
		camera.m_frameCount++;

		// set global and local work item dimensions
		m_szLocalWorkSize[0] = 16;
		m_szLocalWorkSize[1] = 16;
		m_szGlobalWorkSize[0] = shrRoundUp((int)m_szLocalWorkSize[0], m_texture_2d.pitch);
		m_szGlobalWorkSize[1] = shrRoundUp((int)m_szLocalWorkSize[1], m_texture_2d.height);

		// set the args values
		cl_uint argNumber = 0;
		cl_int ciErrNum = clSetKernelArg(m_ckKernel_tex2d, argNumber++, sizeof(m_texture_2d.clTexture), (void *) &(m_texture_2d.clTexture));
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

		cl_mem texture3d = m_textureManager.GetCLTexture3d();
		ciErrNum = clSetKernelArg(m_ckKernel_tex2d, argNumber++, sizeof(texture3d), &texture3d);
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

		CSharedObject<SSharedDataRoot> &sharedDataRoot = SSharedDataRoot::Get();
		ciErrNum = clSetKernelArg(m_ckKernel_tex2d, argNumber++, sizeof(cl_mem), &sharedDataRoot.GetAndUpdateCLMem(m_cxGPUContext, m_cqCommandQueue));
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

		ciErrNum = clSetKernelArg(m_ckKernel_tex2d, argNumber++, sizeof(cl_mem), &m_world.m_pointLights.GetAndUpdateMem(m_cxGPUContext, m_cqCommandQueue));
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

		ciErrNum = clSetKernelArg(m_ckKernel_tex2d, argNumber++, sizeof(cl_mem), &m_world.m_spheres.GetAndUpdateMem(m_cxGPUContext, m_cqCommandQueue));
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

		ciErrNum = clSetKernelArg(m_ckKernel_tex2d, argNumber++, sizeof(cl_mem), &m_world.m_boxes.GetAndUpdateMem(m_cxGPUContext, m_cqCommandQueue));
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

		ciErrNum = clSetKernelArg(m_ckKernel_tex2d, argNumber++, sizeof(cl_mem), &m_world.m_planes.GetAndUpdateMem(m_cxGPUContext, m_cqCommandQueue));
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

		ciErrNum = clSetKernelArg(m_ckKernel_tex2d, argNumber++, sizeof(cl_mem), &m_world.m_sectors.GetAndUpdateMem(m_cxGPUContext, m_cqCommandQueue));
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

		ciErrNum = clSetKernelArg(m_ckKernel_tex2d, argNumber++, sizeof(cl_mem), &m_world.m_materials.GetAndUpdateMem(m_cxGPUContext, m_cqCommandQueue));
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

		ciErrNum = clSetKernelArg(m_ckKernel_tex2d, argNumber++, sizeof(cl_mem), &m_world.m_portals.GetAndUpdateMem(m_cxGPUContext, m_cqCommandQueue));
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);

		// launch computation kernel
		ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQueue, m_ckKernel_tex2d, 2, NULL,
										  m_szGlobalWorkSize, m_szLocalWorkSize, 
										 0, NULL, NULL);
		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);
    }
}

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
static LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
		case WM_INPUT: 
		{
			UINT dwSize = 40;
			static BYTE lpb[40];
    
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 
							lpb, &dwSize, sizeof(RAWINPUTHEADER));
    
			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (GetFocus() == hWnd)
			{    
				if (raw->header.dwType == RIM_TYPEMOUSE) 
				{
					CInput::OnMouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);

					RECT windowRect;
					GetClientRect(hWnd, &windowRect);
					POINT pt;
					pt.x = (windowRect.left + windowRect.right) / 2;
					pt.y = (windowRect.top + windowRect.bottom) / 2;
					ScreenToClient(hWnd, &pt);
					SetCursorPos(pt.x, pt.y);
				} 
				else if (raw->header.dwType == RIM_TYPEKEYBOARD)
				{
					bool pressed = !(raw->data.keyboard.Flags & RI_KEY_BREAK);
					switch(raw->data.keyboard.VKey)
					{
						case VK_ESCAPE: PostQuitMessage(0); exit(1); return 0;

						case 'W': CInput::SetInputToggle(CInput::e_inputToggleWalkForward, pressed); break;
						case 'A': CInput::SetInputToggle(CInput::e_inputToggleWalkLeft, pressed); break;
						case 'S': CInput::SetInputToggle(CInput::e_inputToggleWalkBack, pressed); break;
						case 'D': CInput::SetInputToggle(CInput::e_inputToggleWalkRight, pressed); break;
						case VK_UP: CInput::SetInputToggle(CInput::e_inputToggleWalkForward, pressed); break;
						case VK_DOWN: CInput::SetInputToggle(CInput::e_inputToggleWalkBack, pressed); break;
						case VK_LEFT: CInput::SetInputToggle(CInput::e_inputToggleWalkLeft, pressed); break;
						case VK_RIGHT: CInput::SetInputToggle(CInput::e_inputToggleWalkRight, pressed); break;

						case VK_SPACE: CInput::SetInputToggle(CInput::e_inputToggleJump, pressed); break;

						case 'C': CInput::SetInputToggle(CInput::e_inputToggleCrouch, pressed); break;
						case VK_CONTROL: CInput::SetInputToggle(CInput::e_inputToggleCrouch, pressed); break;

						case 'Z': if (!pressed) CDirectX::Get().ToggleRecording(); break;
					}
				}
			}
			break;
		}
        case WM_DESTROY:
            PostQuitMessage(0);
			exit(1);
            return 0;
        case WM_PAINT:
            ValidateRect(hWnd, NULL);
            return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}





//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
void UpdateFPS(float elapsed);

//-----------------------------------------------------------------------------
// Program main
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	// load graphics settings and get those settings
	CDirectX::Get().LoadGraphicsSettings();
	const SData_GfxSettings& settings = CDirectX::Settings();

	if (argc > 1)
		CDirectX::Get().SetWorld(argv[1]);
	else
		CDirectX::Get().SetWorld(settings.m_DefaultMap.c_str());

	if(!CDirectX::Get().Init((unsigned int)settings.m_Resolution.m_x, (unsigned int)settings.m_Resolution.m_y))
		return 0;

	CGame::Init();

    RAWINPUTDEVICE Rid[2];

	// mouse
    Rid[0].usUsagePage = 0x01; 
    Rid[0].usUsage = 0x02; 
    Rid[0].dwFlags = RIDEV_NOLEGACY;   
	Rid[0].hwndTarget = CDirectX::Get().GetHWND();

	// keyboard
    Rid[1].usUsagePage = 0x01; 
    Rid[1].usUsage = 0x06; 
    Rid[1].dwFlags = RIDEV_NOLEGACY;   
	Rid[1].hwndTarget = CDirectX::Get().GetHWND();

    RegisterRawInputDevices(Rid, 2, sizeof(Rid[0]));

	//
	// the main loop
	//
	while(true) 
	{
		UpdateFPS(0.0f);
		CDirectX::Get().DrawScene(0.0f);
		CGame::Update(0.0f);
		CInput::Update();

		MSG msg;
		ZeroMemory( &msg, sizeof(msg) );
		while( msg.message!=WM_QUIT )
		{
			while( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}

			static DWORD lastTime = 0;
			DWORD newTime = GetTickCount();
			float delta = lastTime > 0 ? (float)(newTime - lastTime) / 1000.0f : 0.0f;
			lastTime = newTime;

			if (CDirectX::Get().IsRecording())
			{
				float wantedDelta = 1.0f/((float)CDirectX::c_recordingFPS);

				if (delta < wantedDelta)
					Sleep((DWORD)((wantedDelta - delta) * 1000.0f));

				delta = wantedDelta;
			}

		    UpdateFPS(delta);
			CDirectX::Get().DrawScene(delta);
			CGame::Update(delta);
			CInput::Update();
		}
    };
}

//-----------------------------------------------------------------------------
void UpdateFPS(float elapsed)
{
	static float time = 0.0f;
	static DWORD frameCount = 0;
	frameCount++;
	time += elapsed;

	if (time > 1.0f)
	{
		float fps = ((float)frameCount / time);
		frameCount = 0;
		time = 0;
		char buffer[256];
		sprintf(buffer, "FPS - %0.2f", fps);
		SetWindowText(CDirectX::Get().GetHWND(), buffer);
	}
}