// NotepadDarkMode.h - Notepad++ style UAH dark mode implementation
//

#pragma once

#include <windows.h>
#include "UAHMenuBar.h"

// Function declarations
bool runUAHWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* lr);
void EnableUAHDarkMode(HWND hWnd);
void CleanupUAHResources();
void PaintDarkMenuBar(HWND hWnd);
void ForceMenuBarRepaint(HWND hWnd);
void SetMenuOwnerDraw(HWND hWnd);

// Color helper functions
HBRUSH GetDlgBackgroundBrush();
HBRUSH GetHotBackgroundBrush();
COLORREF GetTextColor();
COLORREF GetDisabledTextColor();