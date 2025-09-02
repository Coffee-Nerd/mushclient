// DarkMode.cpp : Dark mode support implementation for MUSHclient
// Adapted from Notepad++ implementation
//

#include "stdafx.h"
#include "DarkMode.h"
#include <uxtheme.h>
#include <vssym32.h>

#pragma comment(lib, "uxtheme.lib")

namespace DarkMode
{
    // Global state
    bool g_darkModeSupported = false;
    bool g_darkModeEnabled = false;
    
    // Function pointers for undocumented APIs
    static fnShouldAppsUseDarkMode _ShouldAppsUseDarkMode = nullptr;
    static fnAllowDarkModeForWindow _AllowDarkModeForWindow = nullptr;
    static fnAllowDarkModeForApp _AllowDarkModeForApp = nullptr;
    static fnFlushMenuThemes _FlushMenuThemes = nullptr;
    static fnIsDarkModeAllowedForWindow _IsDarkModeAllowedForWindow = nullptr;
    static fnSetWindowCompositionAttribute _SetWindowCompositionAttribute = nullptr;
    static fnShouldSystemUseDarkMode _ShouldSystemUseDarkMode = nullptr;
    static fnSetPreferredAppMode _SetPreferredAppMode = nullptr;
    static fnIsDarkModeAllowedForApp _IsDarkModeAllowedForApp = nullptr;
    
    // Windows version helpers
    bool IsWindows10()
    {
        OSVERSIONINFOEX osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
        DWORDLONG const dwlConditionMask = VerSetConditionMask(
            VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL),
            VER_MINORVERSION, VER_GREATER_EQUAL);
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) != FALSE;
    }
    
    bool IsWindows11()
    {
        return IsWindows10() && GetWindowsBuildNumber() >= 22000;
    }
    
    DWORD GetWindowsBuildNumber()
    {
        typedef LONG(WINAPI* fnRtlGetVersion)(PRTL_OSVERSIONINFOEXW);
        
        HMODULE hMod = GetModuleHandle(TEXT("ntdll.dll"));
        if (hMod)
        {
            fnRtlGetVersion RtlGetVersion = (fnRtlGetVersion)GetProcAddress(hMod, "RtlGetVersion");
            if (RtlGetVersion)
            {
                RTL_OSVERSIONINFOEXW osvi = { 0 };
                osvi.dwOSVersionInfoSize = sizeof(osvi);
                if (RtlGetVersion(&osvi) == 0)
                {
                    return osvi.dwBuildNumber;
                }
            }
        }
        
        // Fallback: Try to get from registry
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
            _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
            0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            TCHAR szBuild[32] = {0};
            DWORD dwSize = sizeof(szBuild);
            if (RegQueryValueEx(hKey, _T("CurrentBuildNumber"), NULL, NULL, 
                               (LPBYTE)szBuild, &dwSize) == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
                return _ttoi(szBuild);
            }
            RegCloseKey(hKey);
        }
        
        return 0;
    }
    
    bool IsHighContrast()
    {
        HIGHCONTRAST highContrast = { sizeof(highContrast) };
        if (SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, 0))
        {
            return (highContrast.dwFlags & HCF_HIGHCONTRASTON) != 0;
        }
        return false;
    }
    
    bool CheckBuildNumber(DWORD buildNumber)
    {
        return GetWindowsBuildNumber() >= buildNumber;
    }
    
    void LoadDarkModeAPIs()
    {
        HMODULE hUxtheme = LoadLibraryEx(_T("uxtheme.dll"), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (hUxtheme)
        {
            // These ordinals are for Windows 10 1809 and later
            _ShouldAppsUseDarkMode = (fnShouldAppsUseDarkMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(132));
            _AllowDarkModeForWindow = (fnAllowDarkModeForWindow)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133));
            _AllowDarkModeForApp = (fnAllowDarkModeForApp)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
            _FlushMenuThemes = (fnFlushMenuThemes)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(136));
            _IsDarkModeAllowedForWindow = (fnIsDarkModeAllowedForWindow)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(137));
            
            // For Windows 10 1903 and later
            if (CheckBuildNumber(18362))
            {
                _SetPreferredAppMode = (fnSetPreferredAppMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
                _IsDarkModeAllowedForApp = (fnIsDarkModeAllowedForApp)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(139));
            }
            
            // Get SetWindowCompositionAttribute from user32.dll
            HMODULE hUser32 = GetModuleHandle(_T("user32.dll"));
            if (hUser32)
            {
                _SetWindowCompositionAttribute = (fnSetWindowCompositionAttribute)GetProcAddress(hUser32, "SetWindowCompositionAttribute");
            }
            
            // For system dark mode check
            _ShouldSystemUseDarkMode = (fnShouldSystemUseDarkMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(138));
        }
    }
    
    void InitDarkMode()
    {
        LoadDarkModeAPIs();
        
        // Get Windows build number for debugging
        DWORD buildNumber = GetWindowsBuildNumber();
        
        // Check if dark mode is supported (Windows 10 1809 or later)
        g_darkModeSupported = IsWindows10() && CheckBuildNumber(17763) && !IsHighContrast();
        
        #ifdef _DEBUG
        TRACE("Dark Mode Init: Windows 10=%d, Build=%d, Supported=%d\n", 
              IsWindows10() ? 1 : 0, buildNumber, g_darkModeSupported ? 1 : 0);
        TRACE("Dark Mode APIs: SetPreferredAppMode=%p, AllowDarkModeForApp=%p\n",
              _SetPreferredAppMode, _AllowDarkModeForApp);
        #endif
        
        if (g_darkModeSupported)
        {
            // Allow dark mode for app
            if (_SetPreferredAppMode)
            {
                _SetPreferredAppMode(ForceDark);  // Force dark mode
            }
            else if (_AllowDarkModeForApp)
            {
                _AllowDarkModeForApp(true);
            }
            
            // Force dark mode to be enabled for testing
            // Later this can check user preference or system setting
            g_darkModeEnabled = true;  // Force enable for now
            
            #ifdef _DEBUG
            TRACE("Dark Mode Enabled: %d\n", g_darkModeEnabled ? 1 : 0);
            #endif
            
            // Alternative: Check system setting
            // g_darkModeEnabled = ShouldAppsUseDarkMode();
        }
    }
    
    bool ShouldAppsUseDarkMode()
    {
        if (_ShouldAppsUseDarkMode)
        {
            return _ShouldAppsUseDarkMode() != FALSE;
        }
        return false;
    }
    
    bool AllowDarkModeForWindow(HWND hWnd, bool allow)
    {
        if (_AllowDarkModeForWindow)
        {
            return _AllowDarkModeForWindow(hWnd, allow ? TRUE : FALSE) != FALSE;
        }
        return false;
    }
    
    void SetTitleBarThemeColor(HWND hWnd)
    {
        if (!g_darkModeSupported || !g_darkModeEnabled)
            return;
            
        // For Windows 10 version 1903 and later
        if (_SetWindowCompositionAttribute && CheckBuildNumber(18362))
        {
            BOOL dark = g_darkModeEnabled ? TRUE : FALSE;
            WINDOWCOMPOSITIONATTRIBDATA data = { WCA_USEDARKMODECOLORS, &dark, sizeof(dark) };
            _SetWindowCompositionAttribute(hWnd, &data);
        }
        else
        {
            // For older Windows 10 versions, use SetWindowTheme
            if (g_darkModeEnabled)
            {
                SetWindowTheme(hWnd, L"DarkMode_Explorer", nullptr);
            }
            else
            {
                SetWindowTheme(hWnd, nullptr, nullptr);
            }
        }
    }
    
    void RefreshTitleBarThemeColor(HWND hWnd)
    {
        if (!g_darkModeSupported)
            return;
            
        AllowDarkModeForWindow(hWnd, g_darkModeEnabled);
        SetTitleBarThemeColor(hWnd);
        
        // Force redraw
        SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, 
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    
    bool SetDarkMode(bool enable, bool refreshMenuBar)
    {
        if (!g_darkModeSupported)
            return false;
            
        g_darkModeEnabled = enable;
        
        if (refreshMenuBar)
        {
            FlushMenuThemes();
        }
        
        return true;
    }
    
    void FlushMenuThemes()
    {
        if (_FlushMenuThemes)
        {
            _FlushMenuThemes();
        }
    }
    
    void EnableDarkModeForMFCApp()
    {
        if (!g_darkModeSupported || !g_darkModeEnabled)
            return;
            
        // This function can be expanded to handle MFC-specific initialization
        // For now, it ensures the app is ready for dark mode
        if (_SetPreferredAppMode)
        {
            _SetPreferredAppMode(AllowDark);
        }
        else if (_AllowDarkModeForApp)
        {
            _AllowDarkModeForApp(true);
        }
    }
    
    void ApplyDarkModeToMDIFrame(HWND hWnd)
    {
        if (!g_darkModeSupported || !g_darkModeEnabled)
            return;
            
        AllowDarkModeForWindow(hWnd, true);
        SetTitleBarThemeColor(hWnd);
        
        // Set dark theme for common controls
        SetWindowTheme(hWnd, L"DarkMode_Explorer", nullptr);
        
        // Refresh the window
        RefreshTitleBarThemeColor(hWnd);
    }
    
    void ApplyDarkModeToMDIChild(HWND hWnd)
    {
        if (!g_darkModeSupported || !g_darkModeEnabled)
            return;
            
        AllowDarkModeForWindow(hWnd, true);
        
        // MDI child windows may need different handling
        if (CheckBuildNumber(18362))
        {
            SetTitleBarThemeColor(hWnd);
        }
        
        // Set theme for child window
        SetWindowTheme(hWnd, L"DarkMode_Explorer", nullptr);
    }
}