#include "CDirectx.h"
#include <string>
#include <fstream>

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
void Update(float elapsed);

//-----------------------------------------------------------------------------
// Program main
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	if(!CDirectX::Get().Init(1000, 1000))
		return 0;

    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
    Rid[0].dwFlags = RIDEV_INPUTSINK;   
	Rid[0].hwndTarget = CDirectX::Get().GetHWND();
    RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

	//
	// the main loop
	//
	while(true) 
	{
		Update(0.0f);
		CDirectX::Get().DrawScene(0.0f);

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

		    Update(delta);
			CDirectX::Get().DrawScene(delta);
		}
    };
}

//-----------------------------------------------------------------------------
void Update(float elapsed)
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

