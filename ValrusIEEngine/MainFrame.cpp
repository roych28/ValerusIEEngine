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

MainFrame* This = NULL;

TCHAR* szWndTitleMain = _T("valerus ie wrapper");
TCHAR* szWndClassMain = _T("main window wrapper");

static TCHAR __DEBUG_BUF[1024];
#define DLog(fmt, ...)  swprintf(__DEBUG_BUF, fmt, ##__VA_ARGS__); OutputDebugString(__DEBUG_BUF);

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

bool MainFrame::Init()
{
	log_event_log_message(L"MainFrame::Init ->", EVENTLOG_INFORMATION_TYPE,	event_log_source_name);

	if (!RegMainClass())
	{
		log_event_log_message(L"MainFrame::Init RegMainClass failed", EVENTLOG_ERROR_TYPE, event_log_source_name);

		return false;
	}

	hWndMain = CreateWindowEx(0, szWndClassMain,
		szWndTitleMain,
		WS_POPUP | WS_VISIBLE,
		0, 0,
		1, 1,
		NULL, NULL, hInst, NULL);

	SetIEWindowShow(false);

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

	return true;

}

LRESULT CALLBACK MainFrame::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
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
	case WM_DESTROY:
		This->quitThreadPipe = true;
		WaitForSingleObject(This->threadHandle, INFINITE);
		CloseHandle(This->threadHandle);

		log_event_log_message(L"MainFrame::WndProc WM_DESTROY", EVENTLOG_INFORMATION_TYPE, event_log_source_name);
		ExitProcess(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

WORD MainFrame::RegMainClass()
{
	WNDCLASS wc;
	wc.cbClsExtra = wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
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

	if (rcWind.left == -8 && rcWind.top == -8)
	{
		SetWindowPos(hWndMain, HWND_TOP, rcClient.left, (rcWind.bottom) - height, (rcWind.right - rcWind.left) - ptDiff.x, rcClient.bottom - ptDiff.y, SWP_NOREDRAW);
	}
	else
	{
		SetWindowPos(hWndMain, HWND_TOP, 1, (rcWind.bottom - rcWind.top) - height - ptDiff.y, rcClient.right - 1, rcClient.bottom - ptDiff.y, SWP_NOREDRAW);
	}
}

void MainFrame::SetIEWindowShow(bool visible)
{
	ShowWindow(hWndMain, visible ? SW_SHOW : SW_HIDE);
}

//callback that finds chrome and ie windows
BOOL CALLBACK MainFrame::EnumWindowsProc(HWND hwnd, LPARAM lpParam)
{
	TCHAR class_name[512];
	TCHAR title[512];
	GetClassName(hwnd, class_name, sizeof(class_name));
	GetWindowText(hwnd, title, sizeof(title));

	std::wstring titleW = title;
	std::wstring classNameW = class_name;

	if (titleW.find(L"omfoabjiohaoglgimheiknfmmlfdhpke/main.html") != std::string::npos)
	{
		This->hwndChrome = hwnd;		
		SetWindowLong(This->hwndChrome, GWL_STYLE, GetWindowLong(This->hwndChrome, GWL_STYLE) | WS_CLIPCHILDREN);
		SetWindowLong(This->hWndMain, GWL_STYLE, GetWindowLong(This->hWndMain, GWL_STYLE) | WS_CHILD | WS_CLIPCHILDREN);
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
			EnumWindows(MainFrame::EnumWindowsProc, NULL);
		}
		else if (This->parsedValues["command"] == "#ONMOVE#")
		{

		}
		else if (This->parsedValues["command"] == "#INIT#")
		{
			std::string url = This->parsedValues["url"];
			std::wstring urlW = std::wstring(url.begin(), url.end());
			This->SetIEWindowShow(true);
			This->SetIEWindowSize();
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

