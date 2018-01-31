#include "stdafx.h"
#include "MainFrame.h"
#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "WebBrowser.h"
#include "ProcessMessage.h"

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
}

MainFrame::~MainFrame()
{
	delete webBrowser;
}

bool MainFrame::Init()
{
	if (!RegMainClass())
	{
		return false;
	}

	hWndMain = CreateWindowEx(0, szWndClassMain,
		szWndTitleMain,
		WS_POPUP | WS_VISIBLE,
		0, 0,
		99, 99,
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
				This->webBrowser->SetRect(rc);
		}
		break;
	case WM_DESTROY:
		This->quitThreadPipe = true;
		WaitForSingleObject(This->threadHandle, INFINITE);
		CloseHandle(This->threadHandle);
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
		SetWindowPos(hWndMain, HWND_TOP, rcClient.left, (rcWind.bottom - rcWind.top) - height - ptDiff.y, (rcWind.right - rcWind.left) - ptDiff.x, rcClient.bottom - ptDiff.y, SWP_NOREDRAW);
	}
	else
	{
		SetWindowPos(hWndMain, HWND_TOP, 2, (rcWind.bottom - rcWind.top) - height - ptDiff.y * 2, rcClient.right - ptDiff.x / 2 + 1, rcClient.bottom - ptDiff.y, SWP_NOREDRAW);
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

	if (titleW.find(L"biaidifgnfeaniiiemaofgbgflffhgdn/main.html") != std::string::npos)
	{
		This->hwndChrome = hwnd;
		SetWindowLong(This->hwndChrome, GWL_STYLE, GetWindowLong(This->hwndChrome, GWL_STYLE) | WS_CLIPCHILDREN);
		SetWindowLong(This->hWndMain, GWL_STYLE, GetWindowLong(This->hWndMain, GWL_STYLE) | WS_CLIPCHILDREN);
		//SetParent(This->hWndMain, This->hwndChrome);

		return false;
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
			
		}
		else if (This->parsedValues["command"] == "#INIT#")
		{
			std::string url = This->parsedValues["url"];
			std::wstring urlW = std::wstring(url.begin(), url.end());
			EnumWindows(MainFrame::EnumWindowsProc, NULL);
			This->SetIEWindowShow(true);
			This->SetIEWindowSize();
			This->webBrowser->Navigate(urlW);
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
