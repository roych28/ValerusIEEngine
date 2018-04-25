#include "stdafx.h"
#include "MainFrame.h"
#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "WebBrowser.h"
#include "ProcessMessage.h"
#include "EventLog.h"
#include "Utils.h"

MainFrame* This = NULL;

TCHAR* szWndTitleMain = _T("valerus ie wrapper");
TCHAR* szWndClassMain = _T("main window wrapper");

static char __DEBUG_BUF[1024];
#define DLog(fmt, ...)  sprintf(__DEBUG_BUF, fmt, ##__VA_ARGS__); OutputDebugStringA(__DEBUG_BUF);

MainFrame::MainFrame(HINSTANCE hInst)
{
	this->hInst = hInst;
	this->quitThreadPipe = false;
	hwndChrome = NULL;
	threadHandle = INVALID_HANDLE_VALUE;

	This = this;

	log_event_log_message(L"MainFrame Ctor", EVENTLOG_INFORMATION_TYPE,	event_log_source_name);
}

MainFrame::~MainFrame()
{
	delete webBrowser;

	log_event_log_message(L"MainFrame Dtor", EVENTLOG_INFORMATION_TYPE,	event_log_source_name);
}

void updateIERegHKCU(std::wstring key, int val)
{
	std::wstring fullPathKey = L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\" + key;
	HKEY hKey = Utils::OpenKey(HKEY_CURRENT_USER, fullPathKey.c_str());
	Utils::SetIntVal(hKey, L"ValrusIEEngine.exe", val);
	RegCloseKey(hKey);
}

bool MainFrame::Init()
{
	log_event_log_message(L"MainFrame::Init ->", EVENTLOG_INFORMATION_TYPE,	event_log_source_name);

	if (!RegMainClass())
	{
		log_event_log_message(L"MainFrame::Init RegMainClass failed", EVENTLOG_ERROR_TYPE, event_log_source_name);

		return false;
	}

	hWndMain = CreateWindowEx(WS_EX_TOPMOST, szWndClassMain,
		szWndTitleMain,
		WS_POPUP | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInst, NULL);

	SetIEWindowShow(true);

	RECT rcClient;
	GetClientRect(hWndMain, &rcClient);

	webBrowser = new WebBrowser(hWndMain);
	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = rcClient.right;
	rc.bottom = rcClient.bottom;
	webBrowser->SetRect(rc);

	DWORD   dwThreadId;
	threadHandle = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		MainFrame::PipelineThreadFunction,       // thread function name
		this,					// argument to thread function 
		0,                      // use default creation flags 
		&dwThreadId);			// returns the thread identifier 

	updateIERegHKCU(L"FEATURE_BROWSER_EMULATION", 11000);
	updateIERegHKCU(L"FEATURE_96DPI_PIXEL", 1);
	updateIERegHKCU(L"FEATURE_BEHAVIORS", 1);
	updateIERegHKCU(L"FEATURE_DISABLE_MK_PROTOCOL", 1);
	updateIERegHKCU(L"FEATURE_DISABLE_SQM_UPLOAD_FOR_APP", 1);
	updateIERegHKCU(L"FEATURE_INTERNET_SHELL_FOLDERS", 1);
	updateIERegHKCU(L"FEATURE_LEGACY_DLCONTROL_BEHAVIORS", 1);
	updateIERegHKCU(L"FEATURE_LOCALMACHINE_LOCKDOWN", 1);
	updateIERegHKCU(L"FEATURE_MIME_HANDLING", 1);
	updateIERegHKCU(L"FEATURE_MIME_SNIFFING", 1);
	updateIERegHKCU(L"FEATURE_NINPUT_LEGACYMODE", 1);
	updateIERegHKCU(L"FEATURE_OBJECT_CACHING", 1);
	updateIERegHKCU(L"FEATURE_PROTOCOL_LOCKDOWN", 1);
	updateIERegHKCU(L"FEATURE_RESTRICT_ACTIVEXINSTALL", 1);
	updateIERegHKCU(L"FEATURE_SAFE_BINDTOOBJECT", 1);
	updateIERegHKCU(L"FEATURE_SCRIPTURL_MITIGATION", 1);
	updateIERegHKCU(L"FEATURE_SECURITYBAND", 1);

	updateIERegHKCU(L"FEATURE_SPELLCHECKING", 0);
	updateIERegHKCU(L"FEATURE_TABBED_BROWSING", 0);
	updateIERegHKCU(L"FEATURE_USE_WEBOC_OMNAVIGATOR_IMPLEMENTATION", 0);
	
	updateIERegHKCU(L"FEATURE_WEBOC_DOCUMENT_ZOOM", 1);
	updateIERegHKCU(L"FEATURE_WEBOC_POPUPMANAGEMENT", 1);
	updateIERegHKCU(L"FEATURE_WINDOW_RESTRICTIONS", 1);
	updateIERegHKCU(L"FEATURE_XSSFILTER", 1);
	updateIERegHKCU(L"FEATURE_ZONE_ELEVATION", 1);

	return true; 
}

LRESULT CALLBACK MainFrame::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DLog("WndProc %s\r\n", Utils::getMessageAsString(uMsg));
	switch (uMsg)
	{
	case WM_WEB_CONTROL_MESSAGE:
		if(lParam)
		{
			std::wstring url = (wchar_t*)lParam;
			std::string urlA = std::string(url.begin(), url.end());
			std::string message = "{\"text\":\""+ urlA +"\",\"type\":\"#NEW_WINDOW_OPEN#\"}";

			unsigned int len = message.length();

			std::cout << char(len >> 0)
				<< char(len >> 8)
				<< char(len >> 16)
				<< char(len >> 24);

			std::cout << message << std::flush;
		}
		break;
	case WM_SIZE:
		if (This->webBrowser != 0)
		{
			RECT rcClient;
			GetClientRect(This->hWndMain, &rcClient);

			RECT rc;
			rc.left = 0;
			rc.top = 0;
			rc.right = rcClient.right;
			rc.bottom = rcClient.bottom;
			if (This->webBrowser != 0)
			{
				This->webBrowser->SetRect(rc);
				log_event_log_message(L"MainFrame::WndProc WM_SIZE", EVENTLOG_INFORMATION_TYPE, event_log_source_name);
			}				
		}
		break;
	case WM_SHOWWINDOW:
		if (This->webBrowser != 0)
		{
			This->webBrowser->ShowObject();
		}
		break;
	case WM_SETCURSOR:
		break;
	case WM_PAINT:
		break;
	case WM_SYSCOLORCHANGE:
		break;
	case WM_WINDOWPOSCHANGING:
		break;
		
	case WM_DESTROY:
		This->quitThreadPipe = true;
		WaitForSingleObject(This->threadHandle, INFINITE);
		CloseHandle(This->threadHandle);

		log_event_log_message(L"MainFrame::WndProc WM_DESTROY", EVENTLOG_INFORMATION_TYPE, event_log_source_name);
		ExitProcess(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

WORD MainFrame::RegMainClass()
{
	WNDCLASS wc;
	wc.cbClsExtra = wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.hInstance = this->hInst;
	wc.lpfnWndProc = MainFrame::WndProc;
	wc.lpszClassName = szWndClassMain;
	wc.lpszMenuName = NULL;
	wc.style = 0;

	return RegisterClass(&wc);
}

void MainFrame::SetIEWindowSize()
{
	int width = atoi(parsedValues["width"].c_str());
	int height = atoi(parsedValues["height"].c_str());

	RECT rcClient, rcWind;
	POINT ptDiff;
	GetClientRect(hwndChrome, &rcClient);
	GetWindowRect(hwndChrome, &rcWind);
	ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
	ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;

	//SetWindowPos(hwndChrome, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	if (rcWind.left == -8 && rcWind.top == -8)
	{
		SetWindowPos(hWndMain, HWND_TOP, rcClient.left, (rcWind.bottom) - height, (rcWind.right - rcWind.left) - ptDiff.x, rcClient.bottom - ptDiff.y, SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowPos(hWndMain, HWND_TOP, 1, (rcWind.bottom - rcWind.top) - height - ptDiff.y, rcClient.right - 1, rcClient.bottom - ptDiff.y-100, SWP_SHOWWINDOW);
	}

	UpdateWindow(hWndMain);
	UpdateWindow(hwndChrome);
}

void MainFrame::SetIEWindowShow(bool visible)
{
	ShowWindow(hWndMain, visible ? SW_SHOW : SW_HIDE);
	UpdateWindow(hWndMain);
	UpdateWindow(hwndChrome);
}


BOOL CALLBACK MainFrame::EnumWindowsChildrenProc(HWND hwnd, LPARAM lpParam)
{
	TCHAR class_name[512];
	TCHAR title[512];
	GetClassName(hwnd, class_name, sizeof(class_name));
	GetWindowText(hwnd, title, sizeof(title));		
	
	//SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	
	return TRUE;
}

//callback that finds chrome and ie windows
BOOL CALLBACK MainFrame::EnumWindowsProc(HWND hwnd, LPARAM lpParam)
{
	TCHAR class_name[4096];
	TCHAR title[4096];
	GetClassName(hwnd, class_name, sizeof(class_name));
	GetWindowText(hwnd, title, sizeof(title));

	std::wstring titleW = title;
	std::wstring classNameW = class_name;

	if (titleW.find(This->windowTitle) != std::string::npos)
	{
		This->hwndChrome = hwnd;	
		::SetParent(This->hWndMain, This->hwndChrome);
		
		log_event_log_message(L"MainFrame::EnumWindowsProc omfoabjiohaoglgimheiknfmmlfdhpke window founded", EVENTLOG_INFORMATION_TYPE, event_log_source_name);
	}

	return TRUE;
}

DWORD WINAPI MainFrame::PipelineThreadFunction(LPVOID lpParam)
{
	while (!This->quitThreadPipe)
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

		ProcessMessage(msg.c_str(), This->parsedValues);

		std::string message = "{\"text\":\"This is a response message\",\"type\":\"" + This->parsedValues["command"] + "\"}";

		//command exit message
		if (This->parsedValues["command"] == "#STOP#")
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
		else if (This->parsedValues["command"] == "#ONRESIZE#")
		{
			This->SetIEWindowSize();
		}
		else if (This->parsedValues["command"] == "#HIDE#")
		{
			This->SetIEWindowShow(false);
		}
		else if (This->parsedValues["command"] == "#SHOW#")
		{
			This->SetIEWindowShow(true);
		}
		else if (This->parsedValues["command"] == "#CONNECTED#")
		{
			std::string windowTitleA = This->parsedValues["windowTitle"];
			This->windowTitle = std::wstring(windowTitleA.begin(), windowTitleA.end());
			::Sleep(50); //wait for window title to update
			EnumWindows(MainFrame::EnumWindowsProc, NULL);
		}
		else if (This->parsedValues["command"] == "#INIT#")
		{
			std::string url = This->parsedValues["url"];
			std::wstring urlW = std::wstring(url.begin(), url.end());
			This->SetIEWindowShow(true);
			This->SetIEWindowSize();
			
			::UpdateWindow(This->hwndChrome);
			::UpdateWindow(This->hWndMain);
			This->webBrowser->Navigate(urlW);

			log_event_log_message(L"#INIT# navigate to " + urlW, EVENTLOG_INFORMATION_TYPE, event_log_source_name);
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

//HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings\ZoneMap\ProxyBypass
//HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings\ZoneMap\IntranetName
//HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings\ZoneMap\UNCAsIntranet
//HKCU\Software\Microsoft\Windows\CurrentVersion\Internet Settings\ZoneMap\AutoDetect

/*DWORD ps = GetWindowLong(This->hwndChrome, GWL_WNDPROC);
::EnumChildWindows(This->hwndChrome, EnumWindowsChildrenProc, NULL);
//SetWindowLong(This->hwndChrome, GWL_EXSTYLE, GetWindowLong(This->hwndChrome, GWL_EXSTYLE) | WS_EX_LAYERED);

DWORD style = GetWindowLong(This->hWndMain, GWL_STYLE);
style = style & ~(WS_POPUP);
style = style | WS_CHILD;// | WS_CLIPSIBLINGS;
SetWindowLong(This->hWndMain, GWL_STYLE, style);

SetWindowPos(This->hwndChrome, This->hWndMain, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
//HWND topChild = GetTopWindow(This->hwndChrome);
//::ShowWindow(topChild, SW_HIDE);

DWORD chromeThreadId, currentThreadId;
GetWindowThreadProcessId(This->hwndChrome, &chromeThreadId);
GetWindowThreadProcessId(This->hWndMain, &currentThreadId);
BOOL ret = AttachThreadInput(currentThreadId, chromeThreadId, TRUE);*/

/*
else if (This->parsedValues["command"] == "#ONMOVE#")
{

}*/

