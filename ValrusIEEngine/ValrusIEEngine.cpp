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

INT WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	INT nCmdShow) 
{

	OleInitialize(NULL);

	install_event_log_source(event_log_source_name);

	CreateIEProcess();

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