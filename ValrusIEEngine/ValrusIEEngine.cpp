#include "stdafx.h"
#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "WebBrowser.h"
#include "ProcessMessage.h"

HINSTANCE hInst;
HWND hWndMain;
TCHAR* szWndTitleMain = _T("valerus ie wrapper");
TCHAR* szWndClassMain = _T("main window wrapper");

ATOM RegMainClass();
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI MyThreadFunction(LPVOID lpParam);

HWND hWndAddressBar;

#define btnBack 1
#define btnNext 2
#define btnRefresh 3
#define btnGo 4

static TCHAR __DEBUG_BUF[1024];
#define DLog(fmt, ...)  swprintf(__DEBUG_BUF, fmt, ##__VA_ARGS__); OutputDebugString(__DEBUG_BUF);
WebBrowser *webBrowser1;
std::map<std::string, std::string> parsedValues;
HWND hwndChrome = NULL;

INT WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	INT nCmdShow)
{
	OleInitialize(NULL);

	hInst = hInstance;

	if (!RegMainClass())
	{
		MessageBox(NULL, _T("Cannot register main window class"),
			_T("Error No. 1"),
			MB_ICONERROR);
		return 1;
	}

	hWndMain = CreateWindowEx(0, szWndClassMain,
		szWndTitleMain,
		WS_POPUP | WS_VISIBLE,
		0, 0,
		99, 99,
		NULL, NULL, hInst, NULL);

	ShowWindow(hWndMain, SW_HIDE);

	RECT rcClient;
	GetClientRect(hWndMain, &rcClient);

	webBrowser1 = new WebBrowser(hWndMain);
	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = rcClient.right;
	rc.bottom = rcClient.bottom;
	webBrowser1->SetRect(rc);

	DWORD   dwThreadId;
	CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		MyThreadFunction,       // thread function name
		NULL,					// argument to thread function 
		0,                      // use default creation flags 
		&dwThreadId);   // returns the thread identifier 

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}	

	return 0;
}

void SetIEWindowSize()
{

	int width = atoi(parsedValues["width"].c_str());
	int height = atoi(parsedValues["height"].c_str());

	RECT rcClient, rcWind;
	POINT ptDiff;
	GetClientRect(hwndChrome, &rcClient);
	GetWindowRect(hwndChrome, &rcWind);
	ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
	ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;

	SetWindowPos(hWndMain, HWND_TOP, rcClient.left, (rcWind.bottom - rcWind.top) - height - ptDiff.y * 2, (rcWind.right - rcWind.left) - ptDiff.x, rcClient.bottom - ptDiff.y, SWP_NOREDRAW);
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
		SetWindowLong(hwndChrome, GWL_STYLE, GetWindowLong(hwndChrome, GWL_STYLE) | WS_CLIPCHILDREN);
		SetWindowLong(hWndMain, GWL_STYLE, GetWindowLong(hWndMain, GWL_STYLE) | WS_CLIPCHILDREN);
		SetParent(hWndMain, hwndChrome);
	}

	return TRUE;
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
	while (1)
	{
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

		//command exit message
		if (parsedValues["command"] == "#STOP#")
		{
			message = "{\"text\":\"EXITING...\"}";
			int len = message.length();

			std::cout << char(len >> 0)
				<< char(len >> 8)
				<< char(len >> 16)
				<< char(len >> 24);

			std::cout << message;
			ExitProcess(0);
			break;
		}
		else if (parsedValues["command"] == "#HIDE#")
		{
			message = "{\"text\":\"IE Show\"}";

			bool show = parsedValues["show"] == "true" ? true : false;
			//ShowIEWindow(show);
		}
		else if (parsedValues["command"] == "#ONRESIZE#")
		{
			SetIEWindowSize();
		}
		else if (parsedValues["command"] == "#ONMOVE#")
		{
			//SetIEWindowSize();
		}
		else if (parsedValues["command"] == "#INIT#")
		{
			EnumWindows(EnumWindowsProc, NULL);
			SetIEWindowSize();
			ShowWindow(hWndMain, SW_SHOW);
			webBrowser1->Navigate(_T("http://47.21.44.216/"));
		}

		unsigned int len = message.length();

		std::cout << char(len >> 0)
			<< char(len >> 8)
			<< char(len >> 16)
			<< char(len >> 24);

		std::cout << message << std::flush;
	}
	return 0;
}

ATOM RegMainClass()
{
	WNDCLASS wc;
	wc.cbClsExtra = wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.hInstance = hInst;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = szWndClassMain;
	wc.lpszMenuName = NULL;
	wc.style = 0;

	return RegisterClass(&wc);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		/*CreateWindowEx(0, _T("BUTTON"),
			_T("<<< Back"),
			WS_CHILD | WS_VISIBLE,
			5, 5,
			80, 30,
			hWnd, (HMENU)btnBack, hInst, NULL);

		CreateWindowEx(0, _T("BUTTON"),
			_T("Next >>>"),
			WS_CHILD | WS_VISIBLE,
			90, 5,
			80, 30,
			hWnd, (HMENU)btnNext, hInst, NULL);

		CreateWindowEx(0, _T("BUTTON"),
			_T("Refresh"),
			WS_CHILD | WS_VISIBLE,
			175, 5,
			80, 30,
			hWnd, (HMENU)btnRefresh, hInst, NULL);

		hWndAddressBar =
			CreateWindowEx(0, _T("EDIT"),
				_T("http://47.21.44.216/"),
				WS_CHILD | WS_VISIBLE | WS_BORDER,
				260, 10,
				200, 20,
				hWnd, NULL, hInst, NULL);

		CreateWindowEx(0, _T("BUTTON"),
			_T("Go"),
			WS_CHILD | WS_VISIBLE,
			465, 5,
			80, 30,
			hWnd, (HMENU)btnGo, hInst, NULL);*/

		break;
	case WM_COMMAND:
		/*switch (LOWORD(wParam))
		{
		case btnBack:
			webBrowser1->GoBack();
			break;
		case btnNext:
			webBrowser1->GoForward();
			break;
		case btnRefresh:
			webBrowser1->Refresh();
			break;
		case btnGo:
			TCHAR * szUrl = new TCHAR[1024];
			GetWindowText(hWndAddressBar, szUrl, 1024);
			webBrowser1->Navigate(szUrl);
			break;
		}*/
		break;
	case WM_SIZE:
		if (webBrowser1 != 0)
		{
			RECT rcClient;
			GetClientRect(hWndMain, &rcClient);

			RECT rc;
			rc.left = 0;
			rc.top = 0;
			rc.right = rcClient.right;
			rc.bottom = rcClient.bottom;
			if (webBrowser1 != 0)
				webBrowser1->SetRect(rc);
		}
		break;
	case WM_DESTROY:
		ExitProcess(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}
/*#include "stdafx.h"
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

	lstrcpy(lpDirectory, TEXT("C:\\Program Files (x86)\\Internet Explorer"));
	lstrcpy(lpCommandLine, lpDirectory);
	lstrcat(lpCommandLine, TEXT("\\iexplore.exe -k http://47.21.44.216/"));

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
	DWORD procId;
	HANDLE hProc;

	if (hProcessie != INVALID_HANDLE_VALUE)
	{
		if (GetWindowThreadProcessId(hwndie, &procId))
		{
			if (hProc = OpenProcess(PROCESS_TERMINATE, FALSE, procId))
			{
				if (TerminateProcess(hProc, 0))
				{
					DLog(L"THE IE PROCESS CLOSED - success");
				}
				else
				{
					DLog(L"THE IE PROCESS NOT CLOSE");
				}
				CloseHandle(hProc);
			}
			else
				DLog(L"THE IE PROCESS CANT CLOSE - open process failed");
		}
		else
			DLog(L"THE IE PROCESS WAS NOT FOUND");
	}
}

void ShowIEWindow(bool show)
{
	ShowWindow(hwndie, show ? SW_SHOW : SW_HIDE);
}

void SetIEWindowSize()
{

	int width = atoi(parsedValues["width"].c_str());
	int height = atoi(parsedValues["height"].c_str());

	RECT rcClient, rcWind;
	POINT ptDiff;
	GetClientRect(hwndChrome, &rcClient);
	GetWindowRect(hwndChrome, &rcWind);
	ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
	ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;

	SetWindowPos(hwndie, HWND_TOP, rcClient.left, (rcWind.bottom - rcWind.top) - height - ptDiff.y * 2, (rcWind.right - rcWind.left) - ptDiff.x, rcClient.bottom - ptDiff.y, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
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
	}
	else if (titleW == L"Internet Explorer")
	{
		hwndie = hwnd;
		if (hwndie && hwndChrome)
		{
			SetIEWindowSize();
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

		//command exit message
		if (parsedValues["command"] == "#STOP#")
		{
			TerminateIEProcess();
			message = "{\"text\":\"EXITING...\"}";
			int len = message.length();

			std::cout << char(len >> 0)
				<< char(len >> 8)
				<< char(len >> 16)
				<< char(len >> 24);

			std::cout << message;
			break;
		}
		else if (parsedValues["command"] == "#HIDE#")
		{
			message = "{\"text\":\"IE Show\"}";

			bool show = parsedValues["show"] == "true" ? true : false;
			ShowIEWindow(show);
		}
		else if (parsedValues["command"] == "#ONRESIZE#")
		{
			SetIEWindowSize();
		}
		else if (parsedValues["command"] == "#ONMOVE#")
		{
			SetIEWindowSize();
		}
		else if (parsedValues["command"] == "#INIT#")
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
		}

		unsigned int len = message.length();

		std::cout << char(len >> 0)
			<< char(len >> 8)
			<< char(len >> 16)
			<< char(len >> 24);

		std::cout << message << std::flush;
	}

	return 0;
}*/

/*BOOL CALLBACK FindIEEnumWindowsProc(HWND hwnd, LPARAM lparam)
{
//if(!hwnd) return false;
DWORD id = GetWindowThreadProcessId(hwnd, &id);
printf("process id: %d %d\n", id, processId);
if (id == processId)
{
//printf("THEY MATCH!process id: %d\n", id);
char buffer[256];
GetWindowText(hwnd, (LPWSTR)buffer, 255);
hwndie = hwnd;
//printf("%s ", buffer);

}

return true;
}*/

//system("\"C:\\Program Files (x86)\\Internet Explorer\\iexplore.exe\" -k http://47.21.44.216/");
//"\"C:\\Program Files (x86)\\Internet Explorer\\iexplore.exe\" -k http://47.21.44.216/"