/*==================================================================================================

OS.cpp

OS Specific functionality here

==================================================================================================*/

#include "OS.h"

#include "Shlwapi.h"

namespace OS
{
	bool GetAbsolutePath (const char *file, std::string &result)
	{
		// get the absolute path if we can
		TCHAR** fileName={NULL};
		char resultStr[MAX_PATH];
		if (GetFullPathName(file, MAX_PATH, resultStr, fileName) == 0)
			return false;

		// remove the filename portion
		result = resultStr;
		size_t pos = result.find_last_of("\\/");
		if (std::string::npos != pos)
		{
			result = result.substr(0, pos);
			result += "\\";
		}

		// return success
		return true;
	}
};