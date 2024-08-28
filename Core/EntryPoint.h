/*========================================================

 XephTools - Entry Point
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

========================================================*/

#ifndef XE_ENTRYPOINT_H
#define XE_ENTRYPOINT_H

#include <Windows.h>

#include <sstream>
#include <string>
#include <vector>

namespace xe
{
	std::vector<std::wstring> ParseArgs(wchar_t* pCmdLine)
	{
		std::wstring arg;
		std::vector<std::wstring> result;
		bool inQuote = false;

		while (*pCmdLine != 0)
		{
			if (*pCmdLine == L'\"')
			{
				inQuote = !inQuote;
				if (!arg.empty())
				{
					result.push_back(arg);
					arg.clear();
				}
				++pCmdLine;
				continue;
			}
			if (!inQuote && *pCmdLine == L' ')
			{
				if (!arg.empty())
				{
					result.push_back(arg);
					arg.clear();
				}
				++pCmdLine;
				continue;
			}

			arg.push_back(*pCmdLine);
			++pCmdLine;
		}
		return result;
	}

	std::vector<std::wstring> ParseArgs(int argc, wchar_t** argv)
	{
		std::vector<std::wstring> result;
		for (int i = 1; i < argc; ++i)
		{
			result.push_back(argv[i]);
		}
		return result;
	}

	std::vector<std::string> ParseArgs(char* pCmdLine)
	{
		std::string arg;
		std::vector<std::string> result;
		bool inQuote = false;

		while (*pCmdLine != 0)
		{
			if (*pCmdLine == '\"')
			{
				inQuote = !inQuote;
				if (!arg.empty())
				{
					result.push_back(arg);
					arg.clear();
				}
				++pCmdLine;
				continue;
			}
			if (!inQuote && *pCmdLine == ' ')
			{
				if (!arg.empty())
				{
					result.push_back(arg);
					arg.clear();
				}
				++pCmdLine;
				continue;
			}

			arg.push_back(*pCmdLine);
			++pCmdLine;
		}
		return result;
	}

	std::vector<std::string> ParseArgs(int argc, char** argv)
	{
		std::vector<std::string> result;
		for (int i = 1; i < argc; ++i)
		{
			result.push_back(argv[i]);
		}
		return result;
	}
}

#ifdef _CONSOLE
#define EntryPointA int main(int argc, char** argv)
#define EntryPointW int wmain(int argc, wchar_t** argv)
#else // _CONSOLE
#define EntryPointA int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdLine, int cmdshow)
#define EntryPointW int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PWSTR cmdLine, int cmdshow)
#endif //_CONSOLE


//Signature must be
// "int <NAME>(const std::vector<std::string>& args)"
// or
// "int <NAME>(const std::vector<std::wstring>& args)"

#ifndef SetEntryPoint
#ifdef _CONSOLE
#define SetEntryPointA(entry) EntryPointA {\
std::vector<std::string> args = xe::ParseArgs(argc, argv);\
return entry(args);\
}
#define SetEntryPointW(entry) EntryPointW {\
std::vector<std::wstring> args = xe::ParseArgs(argc, argv);\
return entry(args);\
}
#else // _CONSOLE
#define SetEntryPointA(entry) EntryPointA {\
std::vector<std::string> args = xe::ParseArgs(cmdLine);\
return entry(args);\
}
#define SetEntryPointW(entry) EntryPointW {\
std::vector<std::wstring> args = xe::ParseArgs(cmdLine);\
return entry(args);\
}
#endif // _CONSOLE

#ifdef XE_USE_WIDE_ENTRY
#define SetEntryPoint(entry) SetEntryPointW(entry)
#else
#define SetEntryPoint(entry) SetEntryPointA(entry)
#endif //XE_USE_WIDE_ENTRY

#endif //!SetEntryPoint

#endif // !XE_ENTRYPOINT_H