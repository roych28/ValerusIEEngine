// ConsoleApplication1.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <Windows.h>
#include <shlobj.h> 
#include <map>
#include "ProcessMessage.h"

static TCHAR __DEBUG_BUF[1024];
#define DLog(fmt, ...)  swprintf(__DEBUG_BUF, fmt, ##__VA_ARGS__); OutputDebugString(__DEBUG_BUF);

enum PROCESS_STATE
{
	IDLE,
	IE_LOADING,
	IE_UP
};
PROCESS_STATE processState = PROCESS_STATE::IDLE;
HWND hwndChrome = NULL;
HWND hwndie = NULL;
HANDLE hProcessie = INVALID_HANDLE_VALUE;

int browserWidth = -1;
int browserHeight = -1;
DWORD processId = -1L;
std::map<std::string, std::string> parsedValues;

//creates the ie process
void CreateIEProcess()
{
	SECURITY_ATTRIBUTES Security = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	STARTUPINFO si = { sizeof(STARTUPINFO), 0 };
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	TCHAR lpDirectory[MAX_PATH];
	TCHAR lpCommandLine[MAX_PATH];

	// avoid hard-coding your program files directory name. But here I do as you do.
	lstrcpy(lpDirectory, TEXT("C:\\Program Files (x86)\\Internet Explorer"));
	lstrcpy(lpCommandLine, lpDirectory);
	lstrcat(lpCommandLine, TEXT("\\iexplore.exe -k http://47.21.44.216/"));
	//"\"C:\\Program Files (x86)\\Internet Explorer\\iexplore.exe\" -k http://47.21.44.216/"
	if (CreateProcess(
		NULL,  // it will get the app name from the next argument
		lpCommandLine,
		&Security,
		&Security,
		FALSE,  // IE doesn't need your open handles
		CREATE_NEW_PROCESS_GROUP,
		NULL,  // inherit current environment
		NULL,
		&si,
		&pi))
	{
		DWORD res = WaitForSingleObject(pi.hProcess, INFINITE);
		processId = pi.dwProcessId;
		hProcessie = pi.hProcess;
		DLog(L"CreateProcess Success PID %d TID %d", processId, pi.dwThreadId);
	}
}

void TerminateIEProcess()
{
	if (hProcessie != INVALID_HANDLE_VALUE)
	{
		BOOL ret = TerminateProcess(hProcessie, 0);
		DLog(L"TerminateProcess: %d", ret);
		if (ret == 0)
		{
			DWORD errCode = GetLastError();
			DLog(L"TerminateProcess error code: %d", errCode);
		}
		
	}
}

//callback that finds chrome and ie windows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	TCHAR class_name[512];
	TCHAR title[512];
	GetClassName(hwnd, class_name, sizeof(class_name));
	GetWindowText(hwnd, title, sizeof(title));
	
	//DLog(L"Window title: is %ws, Class name is %ws: \n", title, class_name);
	
	std::wstring titleW = title;
	std::wstring classNameW = class_name;
	if (titleW == L"chrome-extension://knldjmfmopnpolahpmmgbagdohdnhkik/main.html - Google Chrome")
	{
		hwndChrome = hwnd;
	}else if (titleW == L"Internet Explorer")
	{
		hwndie = hwnd;
		if (hwndie && hwndChrome)
		{
			RECT parentRect;
			int width = atoi(parsedValues["width"].c_str());
			int height = atoi(parsedValues["height"].c_str());

			GetClientRect(hwndChrome, &parentRect);
			SetWindowPos(hwndie, HWND_TOPMOST, parentRect.left, parentRect.bottom - height, width, height, SWP_SHOWWINDOW);
			SetParent(hwndie, hwndChrome);
		}
	}

	return TRUE;
}

int main() {
	while (1) {
		int length = 0;

		//read the first four bytes (=> Length)
		for (int i = 0; i < 4; i++)
		{
			int read_char = getchar();
			length = length | (read_char << i * 8);
		}

		//read the json-message
		std::string msg = "";
		for (int i = 0; i < (int)length; i++)
		{
			msg += getchar();
		}

		if (length <= 0 || msg.length() <= 0)
		{
			continue;
		}

		ProcessMessage(msg.c_str(), parsedValues);

		std::string message = "{\"text\":\"This is a response message\"}";
		unsigned int len = message.length();

		//command exit message
		if (parsedValues["command"] == "#STOP#") {

			TerminateIEProcess();
			message = "{\"text\":\"EXITING...\"}";
			len = message.length();

			std::cout << char(len >> 0)
				<< char(len >> 8)
				<< char(len >> 16)
				<< char(len >> 24);

			std::cout << message;
			break;
		}
		else
		{
			EnumWindows(EnumWindowsProc, NULL);

			if (hwndChrome && !hwndie)
			{
				CreateIEProcess();
				if (processId != -1L)
				{
					EnumWindows(EnumWindowsProc, NULL);
				}
			}

			std::cout << char(len >> 0)
				<< char(len >> 8)
				<< char(len >> 16)
				<< char(len >> 24);

			std::cout << message << std::flush;
		}	
	}

	return 0;
}

BOOL CALLBACK FindIEEnumWindowsProc(HWND hwnd, LPARAM lparam)
{
	//if(!hwnd) return false;
	DWORD id = GetWindowThreadProcessId(hwnd, &id);
	printf("process id: %d %d\n", id, processId);
	/*if (id == processId)
	{
	//printf("THEY MATCH!process id: %d\n", id);
	char buffer[256];
	GetWindowText(hwnd, (LPWSTR)buffer, 255);
	hwndie = hwnd;
	//printf("%s ", buffer);

	}*/

	return true;
}

//system("\"C:\\Program Files (x86)\\Internet Explorer\\iexplore.exe\" -k http://47.21.44.216/");
