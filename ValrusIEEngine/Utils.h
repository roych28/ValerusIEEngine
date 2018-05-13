#pragma once
#include <Windows.h>


class Utils
{
	
public:
	//static std::map<int, wchar_t*> wmTranslation;
	static const char* getMessageAsString(unsigned int messagenumber);
	static void SetIntVal(HKEY hKey, LPCTSTR lpValue, DWORD data);
	static HKEY OpenKey(HKEY hRootKey, LPCWSTR strKey);
	static std::wstring GetDomainFromURL(std::wstring url);
};

