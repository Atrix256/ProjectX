/*==================================================================================================

Assert.h

==================================================================================================*/

#pragma once

#include <Windows.h>

#define TOSTRING2(x) #x
#define TOSTRING(x) TOSTRING2(x)

#define Assert_(x) { \
	if (!(x)) { \
		OutputDebugStringA("Failed: " TOSTRING(x) " at " __FUNCTION__ " " __FILE__ " line " TOSTRING(__LINE__) "\r\n"); \
		((int*)0)[0] = 0; \
	} \
}

#define AssertI_(x,i) { \
	if (!(x)) { \
		char buffer[2048]; \
		sprintf (buffer, "Failed: " TOSTRING(x) " (%i) at " __FUNCTION__ " " __FILE__ " line " TOSTRING(__LINE__) "\r\n", i); \
		OutputDebugStringA(buffer); \
		((int*)0)[0] = 0; \
	} \
}