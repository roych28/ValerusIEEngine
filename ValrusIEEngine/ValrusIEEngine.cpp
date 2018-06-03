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
		hHTMLWindow = CreateWindow(_T("AtlAxWin"), _T("http://47.21.44.216/"), WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL, 10, 10, 700, 100, hWnd, (HMENU)1000, hInst, 0);
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
#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <Wbemidl.h>
#include "Sddl.h"

#pragma comment(lib, "wbemuuid.lib")

#include "MainFrame.h"
#include "EventLog.h"

BOOL GetLaunchActPermissionsWithIL(SECURITY_DESCRIPTOR *ppSD)
{
	// Allow World Local Launch/Activation permissions. Label the SD for LOW IL Execute UP
	LPWSTR lpszSDDL = L"O:BAG:BAD:(A;;0xb;;;WD)S:(ML;;NX;;;LW)";
	SECURITY_DESCRIPTOR pSD;
	if (ConvertStringSecurityDescriptorToSecurityDescriptorW(lpszSDDL, SDDL_REVISION_1, (PSECURITY_DESCRIPTOR *)&pSD, NULL))
	{
		*ppSD = pSD;
		return TRUE;
	}

	return FALSE;
}

BOOL SetLaunchActPermissions(HKEY hkey, PSECURITY_DESCRIPTOR pSD)
{

	BOOL bResult = FALSE;
	DWORD dwLen = GetSecurityDescriptorLength(pSD);
	LONG lResult;
	lResult = RegSetValueExA(hkey,
		"LaunchPermission",
		0,
		REG_BINARY,
		(BYTE*)pSD,
		dwLen);
	if (lResult != ERROR_SUCCESS) goto done;
	bResult = TRUE;
done:
	return bResult;
};

INT WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	INT nCmdShow) 
{
	OleInitialize(NULL);

	HKEY key = Utils::OpenKey(HKEY_LOCAL_MACHINE, "")
	SetLaunchActPermissions(key, ppsd);
	

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
		if (mf) 
		{
			mf->InjectMessage(&msg);
		}
		DispatchMessage(&msg);
	}	

	delete mf;

	uninstall_event_log_source(event_log_source_name);

	OleUninitialize();

	return 0;
}
*/
/*HRESULT hr = NULL;
	hr = CoInitializeSecurity(
		NULL,							// security descriptor
		-1,                          // use this simple setting
		NULL,                        // use this simple setting
		NULL,                        // reserved
		RPC_C_AUTHN_LEVEL_NONE,   // authentication level  
		RPC_C_IMP_LEVEL_IMPERSONATE, // impersonation level
		NULL,                        // use this simple setting
		EOAC_REQUIRE_FULLSIC,                   // no special capabilities
		NULL);                          // reserved

	if (FAILED(hr))
	{
		CoUninitialize();
		return 1;
	}*/
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

BOOL CreateLowIntegrityProcess(PWSTR pszCommandLine)
{
	DWORD dwError = ERROR_SUCCESS;
	HANDLE hToken = NULL;
	HANDLE hNewToken = NULL;
	SID_IDENTIFIER_AUTHORITY MLAuthority = SECURITY_MANDATORY_LABEL_AUTHORITY;
	PSID pIntegritySid = NULL;
	TOKEN_MANDATORY_LABEL tml = { 0 };
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi = { 0 };

	// Open the primary access token of the process.
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_QUERY |
		TOKEN_ADJUST_DEFAULT | TOKEN_ASSIGN_PRIMARY, &hToken))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Duplicate the primary token of the current process.
	if (!DuplicateTokenEx(hToken, 0, NULL, SecurityImpersonation,
		TokenPrimary, &hNewToken))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Create the low integrity SID.
	if (!AllocateAndInitializeSid(&MLAuthority, 1, SECURITY_MANDATORY_LOW_RID,
		0, 0, 0, 0, 0, 0, 0, &pIntegritySid))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	tml.Label.Attributes = SE_GROUP_INTEGRITY;
	tml.Label.Sid = pIntegritySid;

	// Set the integrity level in the access token to low.
	if (!SetTokenInformation(hNewToken, TokenIntegrityLevel, &tml,
		(sizeof(tml) + GetLengthSid(pIntegritySid))))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Create the new process at the Low integrity level.
	if (!CreateProcessAsUser(hNewToken, NULL, pszCommandLine, NULL, NULL,
		FALSE, 0, NULL, NULL, &si, &pi))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (hToken)
	{
		CloseHandle(hToken);
		hToken = NULL;
	}
	if (hNewToken)
	{
		CloseHandle(hNewToken);
		hNewToken = NULL;
	}
	if (pIntegritySid)
	{
		FreeSid(pIntegritySid);
		pIntegritySid = NULL;
	}
	if (pi.hProcess)
	{
		CloseHandle(pi.hProcess);
		pi.hProcess = NULL;
	}
	if (pi.hThread)
	{
		CloseHandle(pi.hThread);
		pi.hThread = NULL;
	}

	if (ERROR_SUCCESS != dwError)
	{
		// Make sure that the error code is set for failure.
		SetLastError(dwError);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
*/