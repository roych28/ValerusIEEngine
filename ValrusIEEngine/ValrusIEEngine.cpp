#include "stdafx.h"
#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "MainFrame.h"
#include "EventLog.h"

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