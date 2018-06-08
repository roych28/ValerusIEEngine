#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <exdisp.h> // IWebBrowser2

HINSTANCE hInst;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HINSTANCE hATLDLL;
typedef BOOL(__stdcall *PAAWI)(void);
PAAWI pAtlAxWinInit;
typedef HRESULT(__stdcall *PAAGC) (HWND hWnd, IUnknown** pUnknown);
PAAGC pAtlAxGetControl;

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	hATLDLL = LoadLibrary(_T("atl.dll"));
	pAtlAxWinInit = (PAAWI)GetProcAddress(hATLDLL, "AtlAxWinInit");
	if (pAtlAxWinInit)
		pAtlAxWinInit();
	pAtlAxGetControl = (PAAGC)GetProcAddress(hATLDLL, "AtlAxGetControl");
	WNDCLASSEX wcex =
	{
		sizeof(WNDCLASSEX), 0, WndProc, 0, 0, hInst, LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL, TEXT("WindowClass"), NULL,
	};
	if (!RegisterClassEx(&wcex))
		return MessageBox(NULL, L"Cannot register class !", L"Error", MB_ICONERROR | MB_OK);
	int nX = (GetSystemMetrics(SM_CXSCREEN) - 1024) / 2, nY = (GetSystemMetrics(SM_CYSCREEN) - 800) / 2;
	HWND hWnd = CreateWindowEx(0, wcex.lpszClassName, TEXT("Test"), WS_OVERLAPPEDWINDOW, nX, nY, 1024, 800, NULL, NULL, hInst, NULL);
	if (!hWnd)
		return MessageBox(NULL, L"Cannot create window !", L"Error", MB_ICONERROR | MB_OK);
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hHTMLWindow;
	switch (message)
	{
	case WM_CREATE:
	{
		hHTMLWindow = CreateWindow(_T("AtlAxWin"), _T("www.facebook.com"), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL, 10, 10, 700, 100, hWnd, (HMENU)1000, hInst, 0);
		if (hHTMLWindow)
		{
			HRESULT hr = E_FAIL;
			IUnknown* pUnknown = NULL;
			IWebBrowser2 *pWebBrowser = NULL;
			hr = pAtlAxGetControl(hHTMLWindow, &pUnknown);
			if (SUCCEEDED(hr))
			{
				hr = pUnknown->QueryInterface(__uuidof(IWebBrowser2), (void**)&pWebBrowser);
				if (SUCCEEDED(hr))
				{
					// Test
					/*
					BSTR bstrURL;
					VARIANT_BOOL vb;
					bstrURL = SysAllocString(L"www.msdn.com");
					VARIANT var;
					var.vt = VT_EMPTY;
					pWebBrowser->Navigate(bstrURL, &var, &var, &var, &var);
					SysFreeString(bstrURL);
					*/
					pWebBrowser->Release();
				}
			}
		}
		return 0;
	}
	break;
	case WM_SIZE:
		MoveWindow(hHTMLWindow, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
/*#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "MainFrame.h"
#include "EventLog.h"

HANDLE hProcessie = INVALID_HANDLE_VALUE;
DWORD processId = -1L;

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
}

INT WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	INT nCmdShow)
{
	OleInitialize(NULL);

	install_event_log_source(event_log_source_name);

	//CreateIEProcess();

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