#include "stdafx.h"
#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "MainFrame.h"


INT WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	INT nCmdShow)
{
	OleInitialize(NULL);

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

	return 0;
}
