/*==================================================================================================

OS.h

OS Specific functionality here

==================================================================================================*/

#pragma once
#include <string>

namespace OS
{
	bool GetAbsolutePath (const char *file, std::string &result);
};