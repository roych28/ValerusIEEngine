#pragma once
#include <Windows.h>

class HookHwndManager
{
public:
	HookHwndManager();
	~HookHwndManager();

	void HookHwndManager::SetHook(HWND wndToAttach);
	//LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam);
};

