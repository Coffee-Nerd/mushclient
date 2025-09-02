// NotepadDarkMode.cpp - Implementing Notepad++ style UAH dark mode menus
//

#include "stdafx.h"
#include "DarkMode.h"
#include "UAHMenuBar.h"

// Dark mode color constants (similar to Notepad++)
namespace NppColors
{
    // Match the titlebar dark color more closely
    const COLORREF dlgBackground = RGB(32, 32, 32);      // Darker to match titlebar
    const COLORREF hotBackground = RGB(65, 65, 65);      // Lighter gray for hover
    const COLORREF selectedBackground = RGB(80, 80, 80); // Even lighter for selected
    const COLORREF textColor = RGB(240, 240, 240);       // Brighter white text
    const COLORREF disabledTextColor = RGB(120, 120, 120);
    const COLORREF menuBorderColor = RGB(100, 100, 100);
}

// Brushes for menu drawing
static HBRUSH g_dlgBrush = nullptr;
static HBRUSH g_hotBrush = nullptr;

HBRUSH GetDlgBackgroundBrush()
{
    if (!g_dlgBrush)
        g_dlgBrush = CreateSolidBrush(NppColors::dlgBackground);
    return g_dlgBrush;
}

HBRUSH GetHotBackgroundBrush()
{
    if (!g_hotBrush)
        g_hotBrush = CreateSolidBrush(NppColors::hotBackground);
    return g_hotBrush;
}

COLORREF GetTextColor() { return NppColors::textColor; }
COLORREF GetDisabledTextColor() { return NppColors::disabledTextColor; }

// Main UAH Window Procedure - based on Notepad++ implementation
bool runUAHWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* lr)
{
    switch (message)
    {
        case WM_UAHDRAWMENU:
        {
            #ifdef _DEBUG
            TRACE("UAH: WM_UAHDRAWMENU received!\n");
            #endif
            
            auto pUDM = reinterpret_cast<UAHMENU*>(lParam);
            RECT rc{};

            // Get menubar rectangle
            MENUBARINFO mbi{};
            mbi.cbSize = sizeof(MENUBARINFO);
            if (!GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi))
                return false;

            RECT rcWindow{};
            GetWindowRect(hWnd, &rcWindow);

            // Offset menubar rect by window rect
            rc = mbi.rcBar;
            OffsetRect(&rc, -rcWindow.left, -rcWindow.top);
            rc.top -= 1;

            // Fill menubar background with dialog background brush
            FillRect(pUDM->hdc, &rc, GetDlgBackgroundBrush());

            *lr = 0;
            return true;
        }

        case WM_UAHDRAWMENUITEM:
        {
            #ifdef _DEBUG
            TRACE("UAH: WM_UAHDRAWMENUITEM received!\n");
            #endif
            
            auto pUDMI = reinterpret_cast<UAHDRAWMENUITEM*>(lParam);
            
            // Get menu item text
            MENUITEMINFO mii = { sizeof(mii) };
            mii.fMask = MIIM_STRING;
            mii.dwTypeData = nullptr;
            
            if (GetMenuItemInfo(pUDMI->um.hmenu, pUDMI->umi.iPosition, TRUE, &mii))
            {
                if (mii.cch)
                {
                    mii.cch++;
                    std::vector<TCHAR> menuText(mii.cch);
                    mii.dwTypeData = menuText.data();
                    
                    if (GetMenuItemInfo(pUDMI->um.hmenu, pUDMI->umi.iPosition, TRUE, &mii))
                    {
                        // Determine colors based on item state
                        COLORREF textColor = GetTextColor();
                        HBRUSH backgroundBrush = GetDlgBackgroundBrush();
                        
                        // Check if menu item is hot/selected
                        if (pUDMI->dis.itemState & ODS_HOTLIGHT || 
                            pUDMI->dis.itemState & ODS_SELECTED)
                        {
                            backgroundBrush = GetHotBackgroundBrush();
                        }
                        
                        // Check if menu item is disabled
                        if (pUDMI->dis.itemState & ODS_GRAYED ||
                            pUDMI->dis.itemState & ODS_DISABLED)
                        {
                            textColor = GetDisabledTextColor();
                        }
                        
                        // Fill background
                        FillRect(pUDMI->dis.hDC, &pUDMI->dis.rcItem, backgroundBrush);
                        
                        // Draw text
                        SetBkMode(pUDMI->dis.hDC, TRANSPARENT);
                        SetTextColor(pUDMI->dis.hDC, textColor);
                        
                        RECT textRect = pUDMI->dis.rcItem;
                        textRect.left += 8; // Add some padding
                        
                        DrawText(pUDMI->dis.hDC, menuText.data(), -1, &textRect,
                                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                    }
                }
            }
            
            *lr = 0;
            return true;
        }
    }
    
    return false;
}

// Paint dark mode menu bar with complete override
void PaintDarkMenuBar(HWND hWnd)
{
    // Get menu bar info
    MENUBARINFO mbi = {sizeof(mbi)};
    if (!GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi))
        return;
        
    // Get window DC
    HDC hdc = GetWindowDC(hWnd);
    if (!hdc)
        return;
        
    // Get window rectangle
    RECT rcWindow;
    GetWindowRect(hWnd, &rcWindow);
    
    // Calculate menu bar rectangle relative to window
    RECT rcMenu = mbi.rcBar;
    OffsetRect(&rcMenu, -rcWindow.left, -rcWindow.top);
    
    // Fill entire menu bar with dark background first
    FillRect(hdc, &rcMenu, GetDlgBackgroundBrush());
    
    // Get cursor position to determine hover state
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    
    // Convert cursor to window coordinates for menu bar detection
    POINT cursorClient = cursorPos;
    ScreenToClient(hWnd, &cursorClient);
    
    int hoveredItem = -1;
    
    // Draw menu text with proper hover detection
    HMENU hMenu = GetMenu(hWnd);
    if (hMenu)
    {
        int itemCount = GetMenuItemCount(hMenu);
        if (itemCount > 0)
        {
            // Select a font (use system menu font)
            NONCLIENTMETRICS ncm = {sizeof(ncm)};
            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
            {
                HFONT hFont = CreateFontIndirect(&ncm.lfMenuFont);
                HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
                SetBkMode(hdc, TRANSPARENT);
                
                // Check if cursor is in menu bar area first
                bool cursorInMenuBar = PtInRect(&rcMenu, cursorClient);
                
                // First pass: determine which item is hovered
                // Use different strategies for open vs closed menus
                for (int i = 0; i < itemCount; i++)
                {
                    RECT itemRect;
                    if (GetMenuItemRect(hWnd, hMenu, i, &itemRect))
                    {
                        // For open menus, use screen coordinates
                        bool isHovered = PtInRect(&itemRect, cursorPos);
                        
                        // For closed menu bar, also check if we're in the menu bar area
                        // and use a more approximate position calculation
                        if (!isHovered && cursorInMenuBar)
                        {
                            // Convert itemRect to window coordinates for comparison
                            RECT itemRectWindow = itemRect;
                            OffsetRect(&itemRectWindow, -rcWindow.left, -rcWindow.top);
                            isHovered = PtInRect(&itemRectWindow, cursorClient);
                        }
                        
                        if (isHovered)
                        {
                            hoveredItem = i;
                            #ifdef _DEBUG
                            TRACE("Hover detected on item %d at cursor screen(%d,%d) client(%d,%d)\n", 
                                  i, cursorPos.x, cursorPos.y, cursorClient.x, cursorClient.y);
                            #endif
                            break;
                        }
                    }
                }
                
                #ifdef _DEBUG
                static int lastHovered = -2;  // Use -2 to distinguish from -1 (no hover)
                if (hoveredItem != lastHovered)
                {
                    TRACE("Hover state changed: %d -> %d (inMenuBar=%d)\n", 
                          lastHovered, hoveredItem, cursorInMenuBar);
                    lastHovered = hoveredItem;
                }
                #endif
                
                // Second pass: draw each menu item with appropriate background
                for (int i = 0; i < itemCount; i++)
                {
                    RECT itemRect;
                    if (GetMenuItemRect(hWnd, hMenu, i, &itemRect))
                    {
                        // Convert to window coordinates
                        OffsetRect(&itemRect, -rcWindow.left, -rcWindow.top);
                        
                        TCHAR menuText[256];
                        if (GetMenuString(hMenu, i, menuText, 255, MF_BYPOSITION) > 0)
                        {
                            // Check if menu item is enabled
                            UINT menuState = GetMenuState(hMenu, i, MF_BYPOSITION);
                            bool isDisabled = (menuState & MF_GRAYED) || (menuState & MF_DISABLED);
                            
                            // Determine background and text color
                            HBRUSH backgroundBrush = GetDlgBackgroundBrush();
                            COLORREF textColor = isDisabled ? GetDisabledTextColor() : GetTextColor();
                            
                            // Use hover background if this item is hovered
                            if (i == hoveredItem && !isDisabled)
                            {
                                backgroundBrush = GetHotBackgroundBrush();
                            }
                            
                            // Fill background for this specific item
                            FillRect(hdc, &itemRect, backgroundBrush);
                            
                            // Set text color and draw
                            SetTextColor(hdc, textColor);
                            
                            // Keep original text with ampersands for proper mnemonic handling
                            CString displayText(menuText);
                            
                            // Let Windows handle mnemonic display based on system settings
                            UINT drawFlags = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
                            
                            // Check if we should hide mnemonics (normal behavior when menu is closed)
                            BOOL hideAccelerators = FALSE;
                            SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &hideAccelerators, 0);
                            if (hideAccelerators)
                            {
                                drawFlags |= DT_HIDEPREFIX;  // Hide mnemonic underlines
                            }
                            
                            // Draw text with proper mnemonic handling
                            DrawText(hdc, displayText, -1, &itemRect, drawFlags);
                        }
                    }
                }
                
                SelectObject(hdc, hOldFont);
                DeleteObject(hFont);
            }
        }
    }
    
    ReleaseDC(hWnd, hdc);
}

// Force immediate menu bar repaint to override Windows hover states
void ForceMenuBarRepaint(HWND hWnd)
{
    // Invalidate the menu bar area specifically
    MENUBARINFO mbi = {sizeof(mbi)};
    if (GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi))
    {
        RECT rcWindow;
        GetWindowRect(hWnd, &rcWindow);
        RECT rcMenu = mbi.rcBar;
        OffsetRect(&rcMenu, -rcWindow.left, -rcWindow.top);
        
        // Force Windows to repaint this area
        InvalidateRect(hWnd, &rcMenu, FALSE);
        UpdateWindow(hWnd);
        
        // Then immediately paint our dark version over it
        PaintDarkMenuBar(hWnd);
    }
}

// Set menu to owner-drawn to completely control drawing
void SetMenuOwnerDraw(HWND hWnd)
{
    HMENU hMenu = GetMenu(hWnd);
    if (hMenu)
    {
        int itemCount = GetMenuItemCount(hMenu);
        for (int i = 0; i < itemCount; i++)
        {
            MENUITEMINFO mii = {sizeof(mii)};
            mii.fMask = MIIM_FTYPE | MIIM_STRING;
            
            if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
            {
                // Only modify if not already owner-drawn
                if (!(mii.fType & MFT_OWNERDRAW))
                {
                    mii.fType |= MFT_OWNERDRAW;
                    SetMenuItemInfo(hMenu, i, TRUE, &mii);
                }
            }
        }
        DrawMenuBar(hWnd); // Force redraw
    }
}

// Function to enable UAH dark mode for a window
void EnableUAHDarkMode(HWND hWnd)
{
    if (!DarkMode::g_darkModeEnabled)
        return;
        
    // Set menu to owner-drawn for complete control
    SetMenuOwnerDraw(hWnd);
    
    // This function sets up the UAH hook for the window
    // The actual hook is installed via SetWindowLongPtr in the main window
}

// Cleanup function
void CleanupUAHResources()
{
    if (g_dlgBrush)
    {
        DeleteObject(g_dlgBrush);
        g_dlgBrush = nullptr;
    }
    if (g_hotBrush)
    {
        DeleteObject(g_hotBrush);
        g_hotBrush = nullptr;
    }
}