#include "CDirectx.h"
#include "CCamera.h"
#include "SharedGeometry.h"
#include "External\tinyxml\tinyxml2.h"
#include <vector>
#include <string>

#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "dxgi.lib")

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

bool g_keyDownW = false;
bool g_keyDownA = false;
bool g_keyDownS = false;
bool g_keyDownD = false;

bool g_keyDownUp = false;
bool g_keyDownDown = false;
bool g_keyDownLeft = false;
bool g_keyDownRight = false;

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

    if (m_texture_2d.pSRView)
		m_texture_2d.pSRView->Release();

	if (m_texture_2d.pTexture)
		m_texture_2d.pTexture->Release();

	m_pointLights.Release();
	m_spheres.Release();
	m_boxes.Release();
	m_materials.Release();

	if(m_ckKernel_tex2d)
		clReleaseKernel(m_ckKernel_tex2d); 

    if(m_cpProgram_tex2d)
		clReleaseProgram(m_cpProgram_tex2d);

    if(m_cqCommandQueue)
		clReleaseCommandQueue(m_cqCommandQueue);

    if(m_cxGPUContext)
		clReleaseContext(m_cxGPUContext);

	// this causes a crash on exit for some reason??  something to do with cl stuff.
	// if m_clCreateFromD3D10Texture2DKHR is not called, this doesn't crash on exit
	// need to fix (and putting notes in todo)
	//dynlinkUnloadD3D10API();

	// Unregister windows class
	UnregisterClass( m_wc.lpszClassName, m_wc.hInstance );
}

//-----------------------------------------------------------------------------
 bool CDirectX::Init (unsigned int width, unsigned int height)
{
	m_width = width;
	m_height = height;

	if (!dynlinkLoadD3D10API())
	{
		printf("could not dynamicaly link to d3d10\r\n");
		return false;
	}

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

	CreateKernelProgram("texture_2d.cl", "texture_2d.ptx", "cl_kernel_texture_2d", m_cpProgram_tex2d, m_ckKernel_tex2d);

	return S_OK;
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
    m_pSwapChain->Present( 0, 0);
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
        hr = sFnPtr_CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory) );
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
    hr = sFnPtr_D3D10CreateDeviceAndSwapChain( 
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
        hr = sFnPtr_D3D10CompileEffectFromMemory(
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
        
        hr = sFnPtr_D3D10CreateEffectFromMemory(
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
void CDirectX::LoadWorld ()
{
	// clear the existing world data
	m_ambientLight[0] = 0.0f;
	m_ambientLight[1] = 0.0f;
	m_ambientLight[2] = 0.0f;
	m_pointLights.Clear(m_cxGPUContext);
	m_materials.Clear(m_cxGPUContext);
	m_boxes.Clear(m_cxGPUContext);
	m_spheres.Clear(m_cxGPUContext);

	std::vector<std::string> m_materialIDs;

	unsigned int nextObjectId = 1;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(m_world.c_str()) != tinyxml2::XML_NO_ERROR)
		return;

	tinyxml2::XMLElement *world = doc.FirstChildElement("World");
	if (!world)
		return;

	// load ambient light
	{
		tinyxml2::XMLElement *ambientLight = world->FirstChildElement("AmbientLight");
		if (ambientLight)
		{
			const tinyxml2::XMLAttribute *attribute = ambientLight->FirstAttribute();
			while (attribute)
			{
				if (!strcmp(attribute->Name(), "Color"))
					sscanf(attribute->Value(), "%f, %f, %f", &m_ambientLight[0], &m_ambientLight[1], &m_ambientLight[2]);

				attribute = attribute->Next();
			}
		}
	}

	// load point lights
	{
		tinyxml2::XMLElement *lightElement = world->FirstChildElement("PointLight");
		while (lightElement)
		{
			SPointLight &light = m_pointLights.AddOne(m_cxGPUContext);

			const tinyxml2::XMLAttribute *attribute = lightElement->FirstAttribute();
			while (attribute)
			{
				if (!strcmp(attribute->Name(), "Color"))
					sscanf(attribute->Value(), "%f, %f, %f", &light.m_color[0], &light.m_color[1], &light.m_color[2]);
				else if (!strcmp(attribute->Name(), "Position"))
					sscanf(attribute->Value(), "%f, %f, %f", &light.m_position[0], &light.m_position[1], &light.m_position[2]);

				attribute = attribute->Next();
			}

			lightElement = lightElement->NextSiblingElement("PointLight");
		}
	}

	// load materials
	{
		tinyxml2::XMLElement *materialElement = world->FirstChildElement("Material");
		while (materialElement)
		{
			SMaterial &material = m_materials.AddOne(m_cxGPUContext);
			m_materialIDs.push_back("");
			std::string &materialName = m_materialIDs.back();

			const tinyxml2::XMLAttribute *attribute = materialElement->FirstAttribute();
			while (attribute)
			{
				if (!strcmp(attribute->Name(), "id"))
					materialName = attribute->Value();
				else if (!strcmp(attribute->Name(), "DiffuseColor"))
					sscanf(attribute->Value(), "%f, %f, %f", &material.m_diffuseColorAndAmount.s[0], &material.m_diffuseColorAndAmount.s[1], &material.m_diffuseColorAndAmount.s[2]);
				else if (!strcmp(attribute->Name(), "DiffuseAmount"))
					sscanf(attribute->Value(), "%f", &material.m_diffuseColorAndAmount.s[3]);
				else if (!strcmp(attribute->Name(), "SpecularColor"))
					sscanf(attribute->Value(), "%f, %f, %f", &material.m_specularColorAndAmount.s[0], &material.m_specularColorAndAmount.s[1], &material.m_specularColorAndAmount.s[2]);
				else if (!strcmp(attribute->Name(), "SpecularAmount"))
					sscanf(attribute->Value(), "%f", &material.m_specularColorAndAmount.s[3]);
				else if (!strcmp(attribute->Name(), "EmissiveColor"))
					sscanf(attribute->Value(), "%f, %f, %f", &material.m_emissiveColor[0], &material.m_emissiveColor[1], &material.m_emissiveColor[2]);
				else if (!strcmp(attribute->Name(), "ReflectionAmount"))
					sscanf(attribute->Value(), "%f", &material.m_reflectionAmount);
				else if (!strcmp(attribute->Name(), "RefractionIndex"))
					sscanf(attribute->Value(), "%f", &material.m_refractionIndex);
				else if (!strcmp(attribute->Name(), "RefractionAmount"))
					sscanf(attribute->Value(), "%f", &material.m_refractionAmount);

				attribute = attribute->Next();
			}

			materialElement = materialElement->NextSiblingElement("Material");
		}
	}

	// load boxes
	{
		tinyxml2::XMLElement *boxElement = world->FirstChildElement("Box");
		while (boxElement)
		{
			SAABox &box = m_boxes.AddOne(m_cxGPUContext);
			box.m_objectId = ++nextObjectId;

			const tinyxml2::XMLAttribute *attribute = boxElement->FirstAttribute();
			while (attribute)
			{
				if (!strcmp(attribute->Name(), "Position"))
					sscanf(attribute->Value(), "%f, %f, %f", &box.m_position[0], &box.m_position[1], &box.m_position[2]);
				else if (!strcmp(attribute->Name(), "Scale"))
					sscanf(attribute->Value(), "%f, %f, %f", &box.m_scale[0], &box.m_scale[1], &box.m_scale[2]);
				else if (!strcmp(attribute->Name(), "Material"))
				{
					for (unsigned int index = 0; index < m_materialIDs.size(); ++index)
					{
						if (!stricmp(attribute->Value(), m_materialIDs[index].c_str()))
						{
							box.m_materialIndex = index;
							break;
						}
					}
				}
				else if (!strcmp(attribute->Name(), "CastShadows"))
					sscanf(attribute->Value(), "%u", &box.m_castsShadows);

				attribute = attribute->Next();
			}

			boxElement = boxElement->NextSiblingElement("Box");
		}
	}

	// load spheres
	{
		tinyxml2::XMLElement *boxElement = world->FirstChildElement("Sphere");
		while (boxElement)
		{
			SSphere &sphere = m_spheres.AddOne(m_cxGPUContext);
			sphere.m_objectId = ++nextObjectId;

			const tinyxml2::XMLAttribute *attribute = boxElement->FirstAttribute();
			while (attribute)
			{
				if (!strcmp(attribute->Name(), "Position"))
					sscanf(attribute->Value(), "%f, %f, %f", &sphere.m_positionAndRadius.s[0], &sphere.m_positionAndRadius.s[1], &sphere.m_positionAndRadius.s[2]);
				else if (!strcmp(attribute->Name(), "Radius"))
					sscanf(attribute->Value(), "%f", &sphere.m_positionAndRadius.s[3]);
				else if (!strcmp(attribute->Name(), "Material"))
				{
					for (unsigned int index = 0; index < m_materialIDs.size(); ++index)
					{
						if (!stricmp(attribute->Value(), m_materialIDs[index].c_str()))
						{
							sphere.m_materialIndex = index;
							break;
						}
					}
				}
				else if (!strcmp(attribute->Name(), "CastShadows"))
					sscanf(attribute->Value(), "%u", &sphere.m_castsShadows);

				attribute = attribute->Next();
			}

			boxElement = boxElement->NextSiblingElement("Sphere");
		}
	}
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

	LoadWorld();

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

    // build the program
	static char *opts = "-cl-fast-relaxed-math -I ./ -D=OPENCL";
    ciErrNum = clBuildProgram(cpProgram, 0, NULL, opts, NULL, NULL);
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
        1, // tex2d
        memToAcquire,
        0,
        NULL,
        &event);
	//oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
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
        1, // tex2d
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
    static float time = 0.0f;
	time += elapsed;

	const float moveAmount = 5.0f;
	if (g_keyDownW || g_keyDownUp)
		CCamera::Get().MoveForward2D(moveAmount * elapsed);

	if (g_keyDownS || g_keyDownDown)
		CCamera::Get().MoveForward2D(-moveAmount * elapsed);

	if (g_keyDownA || g_keyDownLeft)
		CCamera::Get().MoveLeft(moveAmount * elapsed);

	if (g_keyDownD || g_keyDownRight)
		CCamera::Get().MoveLeft(-moveAmount * elapsed);

	// ----------------------------------------------------------------
    // populate the 2d texture
    {
		// set global and local work item dimensions
		m_szLocalWorkSize[0] = 16;
		m_szLocalWorkSize[1] = 16;
		m_szGlobalWorkSize[0] = shrRoundUp((int)m_szLocalWorkSize[0], m_texture_2d.pitch);
		m_szGlobalWorkSize[1] = shrRoundUp((int)m_szLocalWorkSize[1], m_texture_2d.height);

		SCamera& camera = CCamera::Get().GetCameraData();
		const float cameraViewDistance = CCamera::Get().ViewDistance();
		const float cameraViewWidth = CCamera::Get().ViewWidth();
		const float cameraViewHeight = cameraViewWidth * (float)m_texture_2d.height / (float)m_texture_2d.width;

		// set the args values
		cl_int ciErrNum = clSetKernelArg(m_ckKernel_tex2d, 0, sizeof(m_texture_2d.clTexture), (void *) &(m_texture_2d.clTexture));
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 1, sizeof(camera), &camera);
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 2, sizeof(cameraViewDistance), &cameraViewDistance);
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 3, sizeof(cameraViewWidth), &cameraViewWidth);
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 4, sizeof(cameraViewHeight), &cameraViewHeight);
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 5, sizeof(m_ambientLight), &m_ambientLight);

		cl_int numLights = m_pointLights.Count();
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 6, sizeof(numLights), &numLights);
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 7, sizeof(cl_mem), &m_pointLights.GetAndUpdateMem(m_cqCommandQueue));

		cl_int numSpheres = m_spheres.Count();
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 8, sizeof(numSpheres), &numSpheres);
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 9, sizeof(cl_mem), &m_spheres.GetAndUpdateMem(m_cqCommandQueue));

		cl_int numBoxes = m_boxes.Count();
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 10, sizeof(numBoxes), &numBoxes);
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 11, sizeof(cl_mem), &m_boxes.GetAndUpdateMem(m_cqCommandQueue));

		cl_int numMaterials = m_materials.Count();
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 12, sizeof(numMaterials), &numMaterials);
		ciErrNum |= clSetKernelArg(m_ckKernel_tex2d, 13, sizeof(cl_mem), &m_materials.GetAndUpdateMem(m_cqCommandQueue));

		oclCheckErrorEX(ciErrNum, CL_SUCCESS, NULL);
	    
		// launch computation kernel
		ciErrNum |= clEnqueueNDRangeKernel(m_cqCommandQueue, m_ckKernel_tex2d, 2, NULL,
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
    
			if (raw->header.dwType == RIM_TYPEMOUSE && GetFocus() == hWnd) 
			{
				int xPosRelative = raw->data.mouse.lLastX;
				int yPosRelative = raw->data.mouse.lLastY;

				const float m_mouseSensitivity = 0.01f;

				CCamera::Get().YawRight(-m_mouseSensitivity * (float)xPosRelative);
				CCamera::Get().YawUp(-m_mouseSensitivity * (float)yPosRelative);

				RECT windowRect;
				GetClientRect(hWnd, &windowRect);
				POINT pt;
				pt.x = (windowRect.left + windowRect.right) / 2;
				pt.y = (windowRect.top + windowRect.bottom) / 2;
				ScreenToClient(hWnd, &pt);
				SetCursorPos(pt.x, pt.y);
			} 
			break;
		}
        case WM_KEYUP:
		{
            switch(wParam)
			{
				case 'W': g_keyDownW = false; break;
				case 'A': g_keyDownA = false; break;
				case 'S': g_keyDownS = false; break;
				case 'D': g_keyDownD = false; break;
				case VK_UP: g_keyDownUp = false; break;
				case VK_DOWN: g_keyDownDown = false; break;
				case VK_LEFT: g_keyDownLeft = false; break;
				case VK_RIGHT: g_keyDownRight = false; break;
			}
            break;
		}
        case WM_KEYDOWN:
		{
            switch(wParam)
			{
				case VK_ESCAPE:
				{
					PostQuitMessage(0);
					exit(1);
					return 0;
				}
				case 'W': g_keyDownW = true; break;
				case 'A': g_keyDownA = true; break;
				case 'S': g_keyDownS = true; break;
				case 'D': g_keyDownD = true; break;
				case VK_UP: g_keyDownUp = true; break;
				case VK_DOWN: g_keyDownDown = true; break;
				case VK_LEFT: g_keyDownLeft = true; break;
				case VK_RIGHT: g_keyDownRight = true; break;
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