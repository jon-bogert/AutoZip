/*========================================================

 XephTools - AppData
 Copyright (C) 2022 Jon Bogert (jonbogert@gmail.com)

 This software is provided 'as-is', without any express or implied warranty.
 In no event will the authors be held liable for any damages arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it freely,
 subject to the following restrictions:

 1. The origin of this software must not be misrepresented;
	you must not claim that you wrote the original software.
	If you use this software in a product, an acknowledgment
	in the product documentation would be appreciated but is not required.

 2. Altered source versions must be plainly marked as such,
	and must not be misrepresented as being the original software.

 3. This notice may not be removed or altered from any source distribution.

 USAGE NOTES:
  - Shell32 is required: Add "Shell32.lib" to Linker->Additional Dependencies if using only the header file

========================================================*/


#ifndef XE_APPDATA_H
#define XE_APPDATA_H

#include <ShlObj.h> // IMPORTANT: Must link "Shell32.lib" if using just the header file
#include <iostream>

namespace xe
{
	inline std::string AppDataDir()
	{
		PWSTR appdata = NULL;
		if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &appdata) == S_OK)
		{
			char dest[MAX_PATH];
			size_t numElements;
			wcstombs_s(&numElements, dest, appdata, MAX_PATH);
			dest[numElements] = '\0';
			return std::string(dest);
		}
		else
		{
			std::cerr << "Could not find %APPDATA%" << std::endl;
			return std::string();
		}
	}
}

#define _APPDATA_ xe::AppDataDir()
#define APPDATA_PATH xe::AppDataDir()
#define _APPDATA_CSTR_ xe::AppDataDir().c_str()

#endif // XE_APPDATA_H