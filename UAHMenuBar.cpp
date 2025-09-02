// UAHMenuBar.cpp : Implementation of dark mode menu bar support
//

#include "stdafx.h"
#include "UAHMenuBar.h"
#include "DarkMode.h"

namespace UAHMenuBar
{
    // Window procedure hook for dark mode menu handling
    LRESULT UAHDarkModeWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, WNDPROC pfnWndProc)
    {
        switch (message)
        {
        case WM_UAHDRAWMENU:
            {
                UAHDRAWMENU* pUahDrawMenu = (UAHDRAWMENU*)lParam;
                if (pUahDrawMenu && DarkMode::g_darkModeEnabled)
                {
                    DrawDarkMenuBar(hWnd, pUahDrawMenu);
                    return TRUE;
                }
            }
            break;
            
        case WM_UAHDRAWMENUITEM:
            {
                UAHDRAWMENUITEM* pUahDrawMenuItem = (UAHDRAWMENUITEM*)lParam;
                if (pUahDrawMenuItem && DarkMode::g_darkModeEnabled)
                {
                    DrawDarkMenuItem(hWnd, pUahDrawMenuItem);
                    return TRUE;
                }
            }
            break;
        }
        
        // Call original window procedure
        if (pfnWndProc)
            return CallWindowProc(pfnWndProc, hWnd, message, wParam, lParam);
        else
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    
    void DrawDarkMenuBar(HWND hWnd, UAHDRAWMENU* pUahDrawMenu)
    {
        if (!pUahDrawMenu)
            return;
            
        HDC hdc = pUahDrawMenu->um.hdc;
        RECT& rc = pUahDrawMenu->rc;
        
        // Fill background with dark color
        HBRUSH hBrush = CreateSolidBrush(darkBkgColor);
        FillRect(hdc, &rc, hBrush);
        DeleteObject(hBrush);
    }
    
    void DrawDarkMenuItem(HWND hWnd, UAHDRAWMENUITEM* pUahDrawMenuItem)
    {
        if (!pUahDrawMenuItem)
            return;
            
        DRAWITEMSTRUCT& dis = pUahDrawMenuItem->dis;
        HDC hdc = dis.hDC;
        RECT& rc = dis.rcItem;
        UINT state = dis.itemState;
        
        // Determine colors based on state
        COLORREF bkColor = darkBkgColor;
        COLORREF textColor = darkTextColor;
        
        if (state & ODS_SELECTED)
        {
            bkColor = darkSelectionColor;
        }
        else if (state & ODS_HOTLIGHT)
        {
            bkColor = darkHotBkgColor;
        }
        
        if (state & ODS_DISABLED)
        {
            textColor = darkDisabledTextColor;
        }
        
        // Fill background
        HBRUSH hBrush = CreateSolidBrush(bkColor);
        FillRect(hdc, &rc, hBrush);
        DeleteObject(hBrush);
        
        // Draw text if menu item has text
        if (dis.itemData)
        {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, textColor);
            
            // This is simplified - actual implementation would need to properly
            // extract and draw the menu item text
            MENUITEMINFO mii = {0};
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STRING;
            mii.dwTypeData = NULL;
            
            if (GetMenuItemInfo(pUahDrawMenuItem->um.hmenu, dis.itemID, FALSE, &mii))
            {
                if (mii.cch > 0)
                {
                    mii.cch++;
                    TCHAR* text = new TCHAR[mii.cch];
                    mii.dwTypeData = text;
                    
                    if (GetMenuItemInfo(pUahDrawMenuItem->um.hmenu, dis.itemID, FALSE, &mii))
                    {
                        DrawText(hdc, text, -1, &rc, 
                            DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_EXPANDTABS);
                    }
                    
                    delete[] text;
                }
            }
        }
    }
    
    void PaintDarkMenuNcArea(HWND hWnd, HDC hdc, RECT* pRect)
    {
        if (!DarkMode::g_darkModeEnabled)
            return;
            
        // Paint non-client area of menu popup
        HBRUSH hBrush = CreateSolidBrush(darkBkgColor);
        FillRect(hdc, pRect, hBrush);
        DeleteObject(hBrush);
    }
    
    void ApplyDarkThemeToMenu(HMENU hMenu)
    {
        if (!hMenu || !DarkMode::g_darkModeEnabled)
            return;
            
        MENUINFO mi = {0};
        mi.cbSize = sizeof(MENUINFO);
        mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
        mi.hbrBack = CreateSolidBrush(darkBkgColor);
        SetMenuInfo(hMenu, &mi);
    }
    
    void UpdateMenuBarColors(HWND hWnd)
    {
        if (!DarkMode::g_darkModeEnabled)
            return;
            
        // Force menu bar redraw
        DrawMenuBar(hWnd);
    }
    
    void EnableDarkMenuBarForMFC(HWND hWnd)
    {
        if (!DarkMode::g_darkModeEnabled)
            return;
            
        // Apply dark theme to the menu
        HMENU hMenu = GetMenu(hWnd);
        if (hMenu)
        {
            ApplyDarkThemeToMenu(hMenu);
        }
        
        // Force refresh
        UpdateMenuBarColors(hWnd);
    }
    
    void RefreshMFCMenuBar(HWND hWnd)
    {
        UpdateMenuBarColors(hWnd);
    }
}