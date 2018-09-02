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