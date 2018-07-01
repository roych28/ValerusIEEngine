#include "stdafx.h"
#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "MainFrame.h"
#include "EventLog.h"

HANDLE hProcessie = INVALID_HANDLE_VALUE;
DWORD processId = -1L;

INT WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	INT nCmdShow)
{
	OleInitialize(NULL);

	install_event_log_source(event_log_source_name);

	int timer = 0;
	while (timer < 100000000)
		timer++;
	MainFrame* mf = new MainFrame(hInstance);
	if (!mf->Init())
	{
		MessageBox(NULL, _T("Cannot register main window class"),
			_T("Error No. 1"),
			MB_ICONERROR);
		return 1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		mf->InjectMessage(&msg);
		DispatchMessage(&msg);
	}

	delete mf;

	uninstall_event_log_source(event_log_source_name);

	return 0;
}
/*

log_event_log_message("hello, error",
EVENTLOG_ERROR_TYPE,
event_log_source_name);

log_event_log_message("hello, warning",
EVENTLOG_WARNING_TYPE,
event_log_source_name);

// Uninstall when your application is being uninstalled.
//

*/
/*
void CreateIEProcess()
{
	SECURITY_ATTRIBUTES Security = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	STARTUPINFO si = { sizeof(STARTUPINFO), 0 };
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	TCHAR lpDirectory[MAX_PATH];
	TCHAR lpCommandLine[MAX_PATH];

	// avoid hard-coding your program files directory name. But here I do as you do.
	//lstrcpy(lpDirectory, TEXT("C:\\Program Files (x86)\\Internet Explorer"));
	lstrcpy(lpDirectory, TEXT("C:\\Users\\roych\\AppData\\Local\\IE Tab\\11.3.17.1"));
	lstrcpy(lpCommandLine, lpDirectory);
	lstrcat(lpCommandLine, TEXT("\\ietabhelper.exe"));

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
	}
}*/