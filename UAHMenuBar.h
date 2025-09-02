// UAHMenuBar.h : Undocumented Application Hooks for menu bar dark mode support
// Adapted from Notepad++ implementation
//

#pragma once

#include <windows.h>

// UAH (Undocumented Application Hooks) messages for custom menu drawing
#define WM_UAHDRAWMENU 0x0091
#define WM_UAHDRAWMENUITEM 0x0092
#define WM_UAHINITMENU 0x0093
#define WM_UAHMEASUREMENUITEM 0x0094
#define WM_UAHNCPAINTMENUPOPUPFRAME 0x0095

// UAH structures for menu drawing
typedef struct tagUAHMENU
{
    HMENU hmenu;
    HDC hdc;
    DWORD dwFlags;
} UAHMENU;

typedef struct tagUAHMENUITEM
{
    int iPosition;
    UINT umid;
    UINT umState;
} UAHMENUITEM;

typedef struct tagUAHDRAWMENU
{
    DRAWITEMSTRUCT dis;
    UAHMENU um;
    RECT rc;
} UAHDRAWMENU;

typedef struct tagUAHDRAWMENUITEM
{
    DRAWITEMSTRUCT dis;
    UAHMENU um;
    UAHMENUITEM umi;
} UAHDRAWMENUITEM;

// Flags for menu drawing
#define UAHDM_MENU 0x00000001
#define UAHDM_MENUITEM 0x00000002

namespace UAHMenuBar
{
    // Dark mode colors for menus
    const COLORREF darkBkgColor = RGB(45, 45, 45);
    const COLORREF darkTextColor = RGB(240, 240, 240);
    const COLORREF darkHotBkgColor = RGB(65, 65, 65);
    const COLORREF darkDisabledTextColor = RGB(120, 120, 120);
    const COLORREF darkSelectionColor = RGB(90, 90, 90);
    const COLORREF darkSeparatorColor = RGB(85, 85, 85);
    
    // Function to handle UAH menu messages
    LRESULT UAHDarkModeWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, WNDPROC pfnWndProc);
    
    // Helper functions for dark menu drawing
    void DrawDarkMenuBar(HWND hWnd, UAHDRAWMENU* pUahDrawMenu);
    void DrawDarkMenuItem(HWND hWnd, UAHDRAWMENUITEM* pUahDrawMenuItem);
    void PaintDarkMenuNcArea(HWND hWnd, HDC hdc, RECT* pRect);
    
    // Menu theme application
    void ApplyDarkThemeToMenu(HMENU hMenu);
    void UpdateMenuBarColors(HWND hWnd);
    
    // MFC-specific helpers
    void EnableDarkMenuBarForMFC(HWND hWnd);
    void RefreshMFCMenuBar(HWND hWnd);
}