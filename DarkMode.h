// DarkMode.h : Dark mode support for MUSHclient
// Adapted from Notepad++ implementation
//

#pragma once

#include <windows.h>

namespace DarkMode
{
    // Dark mode state
    extern bool g_darkModeSupported;
    extern bool g_darkModeEnabled;
    
    // Windows version helpers
    bool IsWindows10();
    bool IsWindows11();
    DWORD GetWindowsBuildNumber();
    bool IsHighContrast();
    
    // Core dark mode functions
    void InitDarkMode();
    bool ShouldAppsUseDarkMode();
    bool AllowDarkModeForWindow(HWND hWnd, bool allow);
    void SetTitleBarThemeColor(HWND hWnd);
    void RefreshTitleBarThemeColor(HWND hWnd);
    bool SetDarkMode(bool enable, bool refreshMenuBar = true);
    void FlushMenuThemes();
    
    // MFC-specific helpers
    void EnableDarkModeForMFCApp();
    void ApplyDarkModeToMDIFrame(HWND hWnd);
    void ApplyDarkModeToMDIChild(HWND hWnd);
    
    // Window composition attribute structure
    enum WINDOWCOMPOSITIONATTRIB
    {
        WCA_UNDEFINED = 0,
        WCA_NCRENDERING_ENABLED = 1,
        WCA_NCRENDERING_POLICY = 2,
        WCA_TRANSITIONS_FORCEDISABLED = 3,
        WCA_ALLOW_NCPAINT = 4,
        WCA_CAPTION_BUTTON_BOUNDS = 5,
        WCA_NONCLIENT_RTL_LAYOUT = 6,
        WCA_FORCE_ICONIC_REPRESENTATION = 7,
        WCA_EXTENDED_FRAME_BOUNDS = 8,
        WCA_HAS_ICONIC_BITMAP = 9,
        WCA_THEME_ATTRIBUTES = 10,
        WCA_NCRENDERING_EXILED = 11,
        WCA_NCADORNMENTINFO = 12,
        WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
        WCA_VIDEO_OVERLAY_ACTIVE = 14,
        WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
        WCA_DISALLOW_PEEK = 16,
        WCA_CLOAK = 17,
        WCA_CLOAKED = 18,
        WCA_ACCENT_POLICY = 19,
        WCA_FREEZE_REPRESENTATION = 20,
        WCA_EVER_UNCLOAKED = 21,
        WCA_VISUAL_OWNER = 22,
        WCA_HOLOGRAPHIC = 23,
        WCA_EXCLUDED_FROM_DDA = 24,
        WCA_PASSIVEUPDATEMODE = 25,
        WCA_USEDARKMODECOLORS = 26,
        WCA_LAST = 27
    };

    struct WINDOWCOMPOSITIONATTRIBDATA
    {
        WINDOWCOMPOSITIONATTRIB Attrib;
        PVOID pvData;
        SIZE_T cbData;
    };
    
    // Function pointer types for undocumented APIs
    typedef BOOL (WINAPI *fnShouldAppsUseDarkMode)();
    typedef BOOL (WINAPI *fnAllowDarkModeForWindow)(HWND hWnd, BOOL allow);
    typedef BOOL (WINAPI *fnAllowDarkModeForApp)(BOOL allow);
    typedef void (WINAPI *fnFlushMenuThemes)();
    typedef BOOL (WINAPI *fnIsDarkModeAllowedForWindow)(HWND hWnd);
    typedef BOOL (WINAPI *fnSetWindowCompositionAttribute)(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA*);
    typedef BOOL (WINAPI *fnShouldSystemUseDarkMode)();
    typedef BOOL (WINAPI *fnIsDarkModeAllowedForApp)();
    
    // Preference keys
    enum PreferredAppMode
    {
        Default,
        AllowDark,
        ForceDark,
        ForceLight,
        Max
    };
    
    typedef PreferredAppMode (WINAPI *fnSetPreferredAppMode)(PreferredAppMode appMode);
    
    // Internal helpers
    void LoadDarkModeAPIs();
    bool CheckBuildNumber(DWORD buildNumber);
}