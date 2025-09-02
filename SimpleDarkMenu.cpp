// SimpleDarkMenu.cpp : Simplified dark mode menu implementation
//

#include "stdafx.h"
#include "DarkMode.h"

// Forward declarations
void ApplyDarkModeToSubMenu(HMENU hMenu);

// Function to apply dark mode to menus using Windows 10/11 features
void ApplyDarkModeToMenus(HWND hWnd)
{
    if (!DarkMode::g_darkModeEnabled)
        return;
        
    // Get the menu handle
    HMENU hMenu = ::GetMenu(hWnd);
    if (!hMenu)
        return;
    
    // Windows 10 1903+ supports dark mode for menus via undocumented API
    // But we can use a simpler approach with owner-drawn menus
    
    // Set menu background color
    MENUINFO mi = {0};
    mi.cbSize = sizeof(MENUINFO);
    mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
    mi.hbrBack = CreateSolidBrush(RGB(45, 45, 45));  // Dark background
    ::SetMenuInfo(hMenu, &mi);
    
    // Make all menu items owner-drawn for custom colors
    int count = ::GetMenuItemCount(hMenu);
    for (int i = 0; i < count; i++)
    {
        MENUITEMINFO mii = {0};
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_FTYPE | MIIM_SUBMENU;
        
        if (::GetMenuItemInfo(hMenu, i, TRUE, &mii))
        {
            // If it has a submenu, process it recursively
            if (mii.hSubMenu)
            {
                ApplyDarkModeToSubMenu(mii.hSubMenu);
            }
            
            // Make the item owner-drawn (except separators)
            if (!(mii.fType & MFT_SEPARATOR))
            {
                mii.fMask = MIIM_FTYPE;
                mii.fType |= MFT_OWNERDRAW;
                ::SetMenuItemInfo(hMenu, i, TRUE, &mii);
            }
        }
    }
    
    // Force menu bar redraw
    ::DrawMenuBar(hWnd);
}

void ApplyDarkModeToSubMenu(HMENU hMenu)
{
    if (!hMenu)
        return;
        
    // Set submenu background
    MENUINFO mi = {0};
    mi.cbSize = sizeof(MENUINFO);
    mi.fMask = MIM_BACKGROUND;
    mi.hbrBack = CreateSolidBrush(RGB(45, 45, 45));
    ::SetMenuInfo(hMenu, &mi);
    
    // Process submenu items
    int count = ::GetMenuItemCount(hMenu);
    for (int i = 0; i < count; i++)
    {
        MENUITEMINFO mii = {0};
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_FTYPE | MIIM_SUBMENU;
        
        if (::GetMenuItemInfo(hMenu, i, TRUE, &mii))
        {
            // Recursively process nested submenus
            if (mii.hSubMenu)
            {
                ApplyDarkModeToSubMenu(mii.hSubMenu);
            }
            
            // Make items owner-drawn
            if (!(mii.fType & MFT_SEPARATOR))
            {
                mii.fMask = MIIM_FTYPE;
                mii.fType |= MFT_OWNERDRAW;
                ::SetMenuItemInfo(hMenu, i, TRUE, &mii);
            }
        }
    }
}

// Alternative: Try to use Windows' built-in dark mode for context menus
void EnableBuiltInDarkModeForMenus(HWND hWnd)
{
    // This uses the AllowDarkModeForWindow which might affect menus
    if (DarkMode::g_darkModeEnabled)
    {
        // Try to apply dark mode using the window theme
        SetWindowTheme(hWnd, L"DarkMode_Explorer", NULL);
        
        // Get menu and try to theme it
        HMENU hMenu = ::GetMenu(hWnd);
        if (hMenu)
        {
            // Try to set dark theme on menu window (if it exists)
            // Note: Menu windows are created on-demand, so this might not work immediately
            HWND hMenuWnd = ::FindWindow(_T("#32768"), NULL);  // Menu window class
            if (hMenuWnd)
            {
                SetWindowTheme(hMenuWnd, L"DarkMode_Explorer", NULL);
            }
        }
    }
}