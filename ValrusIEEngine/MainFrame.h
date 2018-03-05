#pragma once
#include <Windows.h>

class WebBrowser;

class MainFrame
{
private:
	HINSTANCE hInst;
	std::wstring windowTitle;
	HWND hWndMain;
	BOOL quitThreadPipe;
	HWND hwndChrome;
	HANDLE threadHandle;
	WebBrowser *webBrowser;
	std::map<std::string, std::string> parsedValues;

	ATOM RegMainClass();

	static DWORD WINAPI  PipelineThreadFunction(LPVOID lpParam);
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lpParam);
	static BOOL CALLBACK EnumWindowsChildrenProc(HWND hwnd, LPARAM lpParam);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	void SetIEWindowSize();
	void SetIEWindowShow(bool visible);
public:
	MainFrame(HINSTANCE hInst);
	~MainFrame();

	bool Init();
};

