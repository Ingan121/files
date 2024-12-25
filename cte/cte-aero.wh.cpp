// ==WindhawkMod==
// @id              cef-titlebar-enabler-universal
// @name            CEF/Spotify Titlebar Enabler
// @description     Force native frames and title bars for CEF apps
// @version         0.4
// @author          Ingan121
// @github          https://github.com/Ingan121
// @twitter         https://twitter.com/Ingan121
// @homepage        https://www.ingan121.com/
// @include         spotify.exe
// @include         cefclient.exe
// @compilerOptions -lcomctl32 -luxtheme
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# CEF Titlebar Enabler
* Force native frames and title bars for CEF apps, such as Spotify
* Only works on apps using native CEF top-level windows
    * Steam uses SDL for its top-level windows (except DevTools), so this mod doesn't work with Steam
* Electron apps are NOT supported! Just patch asar to override `frame: false` to true in BrowserWindow creation
* Supported CEF versions: 90.4 to 132
    * This mod won't work with versions before 90.4
    * Versions after 132 may work but are not tested
    * Variant of this mod using copy-pasted CEF structs instead of hardcoded offsets is available at [here](https://github.com/Ingan121/files/tree/master/cte)
    * Copy required structs/definitions from your wanted CEF version (available [here](https://cef-builds.spotifycdn.com/index.html)) and paste them to the above variant to calculate the offsets
    * Testing with cefclient: `cefclient.exe --use-views --hide-frame --hide-controls`
* Supported Spotify versions: 1.1.60 to 1.2.53 (newer versions may work)
* Spotify notes:
    * Old releases are available [here](https://docs.google.com/spreadsheets/d/1wztO1L4zvNykBRw7X4jxP8pvo11oQjT0O5DvZ_-S4Ok/edit?pli=1&gid=803394557#gid=803394557)
    * 1.1.60-1.1.67: Use [SpotifyNoControl](https://github.com/JulienMaille/SpotifyNoControl) to remove the window controls
    * 1.1.68-1.1.70: Window control hiding doesn't work
    * 1.2.7: First version to use Library X UI by default
    * 1.2.13: Last version to have the old UI
    * 1.2.28: First version to support Chrome runtime (disabled by default)
    * 1.2.45: Last version to support disabling the global navbar
    * 1.2.47: Chrome runtime is always enabled since this version
    * Try the [noControls](https://github.com/ohitstom/spicetify-extensions/tree/main/noControls) Spicetify extension to remove the empty space left by the custom window controls
    * Enable Chrome runtime to get a proper window icon. Use `--enable-chrome-runtime` flag or put `app.enable-chrome-runtime=true` in `%appdata%\Spotify\prefs`
    * Spicetify extension developers: Use `window.outerHeight - window.innerHeight > 0` to detect if the window has a native title bar
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- showframe: true
  $name: Enable native frames and title bars on the main window*
  $description: "(*): Requires a restart to take effect"
- showframeonothers: false
  $name: Enable native frames and title bars on other windows
  $description: Includes Miniplayer, DevTools, etc.
- showmenu: true
  $name: Show the menu button*
  $description: Disabling this also prevents opening the Spotify menu with the Alt key
- showcontrols: false
  $name: Show Spotify's custom window controls*
- transparentcontrols: false
  $name: Make Spotify's custom window controls transparent
- ignoreminsize: false
  $name: Ignore minimum window size
  $description: Allows resizing the window below the minimum size set by Spotify
- allowuntested: false
  $name: (Advanced) Use unsafe methods on untested CEF versions
  $description: Allows calling unsafe functions on untested CEF versions. May cause crashes or other issues. If disabled, an inefficient alternative method will be used on untested versions.
*/
// ==/WindhawkModSettings==

/* Spotify CEF version map
90.6: 1.1.60-1.1.62
91.1: 1.1.63-1.1.67
91.3: 1.1.68-1.1.70
94: 1.1.71
95: 1.1.74-1.1.75
96: 1.1.76
98: 1.1.81
100: 1.1.85
101: 1.1.86-1.1.88
102: 1.1.89
104: 1.1.94
106: 1.1.97-1.2.3
109: 1.2.4-1.2.6
110: 1.2.7
111: 1.2.8-1.2.10
112: 1.2.11-1.2.12
113: 1.2.13-1.2.19
115: 1.2.20
116: 1.2.21-1.2.22
117: 1.2.23-1.2.24
118: 1.2.25
119: 1.2.26
120: 1.2.28-1.2.30
121: 1.2.31-1.2.32
122: 1.2.33-1.2.37
124: 1.2.38-1.2.39
125: 1.2.40-1.2.44
127: 1.2.45-1.2.46
128: 1.2.47-1.2.48
129: 1.2.49-1.2.50
130: 1.2.51-1.2.52
131: 1.2.53
*/

#include <libloaderapi.h>
#include <windhawk_utils.h>
#include <uxtheme.h>
#include <windows.h>

#define CEF_CALLBACK __stdcall
#define CEF_EXPORT __cdecl
#define cef_window_handle_t HWND
#define ANY_MINOR -1

typedef uint32_t cef_color_t;
typedef uint32_t SkColor;

struct cte_settings {
    BOOL showframe;
    BOOL showframeonothers;
    BOOL showmenu;
    BOOL showcontrols;
    BOOL transparentcontrols;
    BOOL ignoreminsize;
    BOOL allowuntested;
} cte_settings;

typedef struct cte_offset {
  int ver_major;
  int ver_minor; // -1 for any
  int offset_x86;
  int offset_x64;
} cte_offset_t;

cte_offset_t is_frameless_offsets[] = {
    {90, 4, 0x48, 0x90},
    {90, 5, 0x48, 0x90},
    {90, 6, 0x48, 0x90},
    {91, 0, 0x48, 0x90},
    {91, 1, 0x48, 0x90},
    // (91.2 is found nowhere)
    {91, 3, 0x50, 0xa0},
    {92, ANY_MINOR, 0x50, 0xa0},
    {101, ANY_MINOR, 0x50, 0xa0},
    {102, ANY_MINOR, 0x54, 0xa8},
    {107, ANY_MINOR, 0x54, 0xa8},
    {108, ANY_MINOR, 0x5c, 0xb8},
    {114, ANY_MINOR, 0x5c, 0xb8},
    {115, ANY_MINOR, 0x60, 0xc0},
    {116, ANY_MINOR, 0x60, 0xc0},
    {117, ANY_MINOR, 0x64, 0xc8},
    {123, ANY_MINOR, 0x64, 0xc8},
    {124, ANY_MINOR, 0x68, 0xd0},
    {132, ANY_MINOR, 0x68, 0xd0}
};

cte_offset_t add_child_view_offsets[] = {
    {94, ANY_MINOR, 0xf0, 0x1e0},
    {122, ANY_MINOR, 0xf0, 0x1e0},
    {124, ANY_MINOR, 0xf4, 0x1e8},
    {130, ANY_MINOR, 0xf4, 0x1e8},
    {131, ANY_MINOR, 0xf8, 0x1f0},
    {132, ANY_MINOR, 0xf8, 0x1f0}
};

cte_offset_t get_window_handle_offsets[] = {
    {94, ANY_MINOR, 0x184, 0x308},
    {114, ANY_MINOR, 0x184, 0x308},
    {115, ANY_MINOR, 0x188, 0x310},
    {123, ANY_MINOR, 0x188, 0x310},
    {124, ANY_MINOR, 0x18c, 0x318},
    {130, ANY_MINOR, 0x18c, 0x318},
    {131, ANY_MINOR, 0x194, 0x328},
    {132, ANY_MINOR, 0x194, 0x328}
};

cte_offset_t set_background_color_offsets[] = {
    {127, ANY_MINOR, NULL, 0x178}
};

int is_frameless_offset = NULL;
int add_child_view_offset = NULL;
int get_window_handle_offset = NULL;
int set_background_color_offset = NULL;

// Same offset for all versions that supports window control hiding
// Cuz get_preferred_size is the very first function in the struct (cef_panel_delegate_t->(cef_view_delegate_t)base.get_preferred_size)
// And cef_base_ref_counted_t, which is the base struct of cef_view_delegate_t, hasn't changed since 94
#ifdef _WIN64
    int get_preferred_size_offset = 0x28;
#else
    int get_preferred_size_offset = 0x14;
#endif

LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DWORD_PTR dwRefData) {
    // dwRefData is 1 if the window is created by cef_window_create_top_level
    // Assumed 1 if this mod is loaded after the window is created
    // dwRefData is 2 if the window is created by cef_window_create_top_level and is_frameless is hooked
    switch (uMsg) {
        case WM_NCHITTEST:
        case WM_NCLBUTTONDOWN:
        case WM_NCPAINT:
        case WM_NCCALCSIZE:
            // Unhook Spotify's custom window control event handling
            // Also unhook WM_NCPAINT to fix non-DWM frames randomly going black
            // WM_NCCALCSIZE is only for windows with Chrome's custom frame (DevTools, Miniplayer, full browser UI, etc.)
            if (dwRefData) {
                if (cte_settings.showframe == TRUE || dwRefData == 2) {
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
                }
            } else if (cte_settings.showframeonothers == TRUE) {
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
            break;
        case WM_GETMINMAXINFO:
            if (cte_settings.ignoreminsize == TRUE) {
                // Ignore minimum window size
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
            break;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK UpdateEnumWindowsProc(HWND hWnd, LPARAM lParam) {
    DWORD pid;
    GetWindowThreadProcessId(hWnd, &pid);
    if (pid == GetCurrentProcessId()) {
        // Update NonClient size
        wchar_t className[256];
        GetClassName(hWnd, className, 256);
        if (wcsncmp(className, L"Chrome_WidgetWin_", 17) == 0) {
            if (lParam == 1) {
                // Really move the window a bit to make Spotify update window control colors
                RECT rect;
                GetWindowRect(hWnd, &rect);
                SetWindowPos(hWnd, NULL, rect.left, rect.top + 1, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
                SetWindowPos(hWnd, NULL, rect.left, rect.top, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
            } else {
                SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
            }
        }
    }
    return TRUE;
}

BOOL CALLBACK InitEnumWindowsProc(HWND hWnd, LPARAM lParam) {
    DWORD pid;
    GetWindowThreadProcessId(hWnd, &pid);
    // Subclass all relevant windows belonging to this process
    if (pid == GetCurrentProcessId()) {
        wchar_t className[256];
        GetClassName(hWnd, className, 256);
        if (wcsncmp(className, L"Chrome_WidgetWin_", 17) == 0) {
            if (WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd, SubclassProc, 1)) {
                Wh_Log(L"Subclassed %p", hWnd);
                if (lParam == 1) {
                    UpdateEnumWindowsProc(hWnd, 0);
                }
            }
        }
    }
    return TRUE;
}

BOOL CALLBACK UninitEnumWindowsProc(HWND hWnd, LPARAM lParam) {
    DWORD pid;
    GetWindowThreadProcessId(hWnd, &pid);
    // Unsubclass all windows belonging to this process
    if (pid == GetCurrentProcessId()) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd, SubclassProc);
        if (lParam == 1) {
            UpdateEnumWindowsProc(hWnd, 0);
        }
    }
    return TRUE;
}

typedef int CEF_CALLBACK (*is_frameless_t)(struct _cef_window_delegate_t* self, struct _cef_window_t* window);
int CEF_CALLBACK is_frameless_hook(struct _cef_window_delegate_t* self, struct _cef_window_t* window) {
    Wh_Log(L"is_frameless_hook");
    return 0;
}

typedef cef_window_handle_t CEF_CALLBACK (*get_window_handle_t)(struct _cef_window_t* self);

typedef _cef_window_t* CEF_EXPORT (*cef_window_create_top_level_t)(void* delegate);
cef_window_create_top_level_t CEF_EXPORT cef_window_create_top_level_original;
_cef_window_t* CEF_EXPORT cef_window_create_top_level_hook(void* delegate) {
    Wh_Log(L"cef_window_create_top_level_hook");

    BOOL is_frameless_hooked = FALSE;
    if (is_frameless_offset != NULL && cte_settings.showframe == TRUE) {
        *((is_frameless_t*)((char*)delegate + is_frameless_offset)) = is_frameless_hook;
        is_frameless_hooked = TRUE;
    }
    _cef_window_t* window = cef_window_create_top_level_original(delegate);
    if (get_window_handle_offset != NULL) {
        get_window_handle_t get_window_handle = *((get_window_handle_t*)((char*)window + get_window_handle_offset));
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(get_window_handle(window), SubclassProc);
        if (WindhawkUtils::SetWindowSubclassFromAnyThread(get_window_handle(window), SubclassProc, is_frameless_hooked ? 2 : 1)) {
            Wh_Log(L"Subclassed %p", get_window_handle(window));
        }
    } else {
        // Just subclass everything again if on an untested version
        // Calling functions from invalid offsets will crash the app for sure
        EnumWindows(UninitEnumWindowsProc, 0);
        EnumWindows(InitEnumWindowsProc, 1);
        Wh_Log(L"Avoided calling get_window_handle on an untested version");
    }
    return window;
}

typedef void CEF_CALLBACK (*set_background_color_t)(struct _cef_view_t* self, cef_color_t color);
set_background_color_t CEF_CALLBACK set_background_color_original;
void CEF_CALLBACK set_background_color_hook(struct _cef_view_t* self, cef_color_t color) {
    //Wh_Log(L"set_background_color_hook: %#x", color);
    // 0x87000000: normal, 0x3fffffff: hover, 0x33ffffff: active, 0xffc42b1c: close button hover, 0xff941320: close button active
    if (color == 0x87000000 && cte_settings.transparentcontrols == TRUE) {
        color = 0x00000000;
    }
    set_background_color_original(self, color);
    return;
}

struct cte_control_container {
    set_background_color_t CEF_CALLBACK set_background_color_original;
    set_background_color_t* CEF_CALLBACK set_background_color_addr;
} cte_controls[3];

int cnt = -1;

typedef void CEF_CALLBACK (*add_child_view_t)(struct _cef_panel_t* self, struct _cef_view_t* view);
add_child_view_t CEF_CALLBACK add_child_view_original;
void CEF_CALLBACK add_child_view_hook(struct _cef_panel_t* self, struct _cef_view_t* view) {
    cnt++;
    Wh_Log(L"add_child_view_hook: %d", cnt);
    // 0: Minimize, 1: Maximize, 2: Close, 3: Menu (removing this also prevents alt key from working)
    if (cnt < 3) {
      if (cte_settings.showcontrols == FALSE) {
        return;
      }
    } else if (cte_settings.showmenu == FALSE) {
      return;
    }
    if (cnt < 3 && set_background_color_offset != NULL) {
        set_background_color_original = *((set_background_color_t*)((char*)view + set_background_color_offset));
        *((set_background_color_t*)((char*)view + set_background_color_offset)) = set_background_color_hook;
        cte_controls[cnt].set_background_color_original = set_background_color_original;
        cte_controls[cnt].set_background_color_addr = (set_background_color_t*)((char*)view + set_background_color_offset);
    }
    add_child_view_original(self, view);
    return;
}

typedef _cef_panel_t* CEF_EXPORT (*cef_panel_create_t)(void* delegate);
cef_panel_create_t CEF_EXPORT cef_panel_create_original;
_cef_panel_t* CEF_EXPORT cef_panel_create_hook(void* delegate) {
    Wh_Log(L"cef_panel_create_hook");
    if ((cnt != 2 || cte_settings.showmenu == FALSE) && // left panel
        (cnt != -1 || cte_settings.showcontrols == FALSE) // right panel
    ) {
        // Nullify get_preferred_size to make the leftover space from hiding the window controls clickable
        // This has side effect of making the menu button ignore the height set by cosmos endpoint (used by noControls Spicetify extension)
        // So only nullify get_preferred_size for the left panel if menu button is hidden
        *((void**)((char*)delegate + get_preferred_size_offset)) = NULL;
    }
    _cef_panel_t* panel = cef_panel_create_original(delegate);
    if (add_child_view_offset != NULL) {
        add_child_view_original = *((add_child_view_t*)((char*)panel + add_child_view_offset));
        *((add_child_view_t*)((char*)panel + add_child_view_offset)) = add_child_view_hook;
    }
    return panel;
}

using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_original;
HWND WINAPI CreateWindowExW_hook(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    Wh_Log(L"CreateWindowExW_hook");
    HWND hWnd = CreateWindowExW_original(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    if (hWnd != NULL) {
        wchar_t className[256];
        GetClassName(hWnd, className, 256);
        if (wcsncmp(className, L"Chrome_WidgetWin_", 17) == 0) { // Chrome_WidgetWin_1: with Chrome runtime, Chrome_WidgetWin_0: without Chrome runtime (Alloy) + some hidden windows
            if (dwStyle & WS_CAPTION) {
                // Subclass other Chromium/CEF windows, including those not created by cef_window_create_top_level (e.g. DevTools, Miniplayer (DocumentPictureInPicture), full Chromium browser UI that somehow can be opened, etc.)
                // But exclude windows without WS_CAPTION to prevent subclassing dropdowns, tooltips, etc.
                if (WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd, SubclassProc, 0)) {
                    Wh_Log(L"Subclassed %p", hWnd);
                }
            }
        }
    }
    return hWnd;
}

using SetWindowThemeAttribute_t = decltype(&SetWindowThemeAttribute);
SetWindowThemeAttribute_t SetWindowThemeAttribute_original;
HRESULT WINAPI SetWindowThemeAttribute_hook(HWND hwnd, enum WINDOWTHEMEATTRIBUTETYPE eAttribute, PVOID pvAttribute, DWORD cbAttribute) {
    Wh_Log(L"SetWindowThemeAttribute_hook");
    if (eAttribute == WTA_NONCLIENT && is_frameless_offset != NULL && cte_settings.showframe == TRUE) {
        // Ignore this to make sure DWM window controls are visible
        return S_OK;
    } else {
        return SetWindowThemeAttribute_original(hwnd, eAttribute, pvAttribute, cbAttribute);
    }
}

// #pragma region "Code section scan for hooking without symbols"
// // From Chrome UI Tweaks by VasherMC

// typedef struct {
//     std::string_view search; // instructions to search for
//     std::string_view prologue; // prologue of the function to be hooked (instructions at entry point)
//     const size_t instr_offset; // estimated location of the searched instructions relative to the entry point
// } function_search;

// // Wrapper for string_view::find, that checks the needle is unique within the haystack.
// const char* unique_search(std::string_view haystack, std::string_view needle, LPCWSTR symbol_name) {
//     size_t index1 = haystack.find(needle);
//     if (index1 == std::string_view::npos) {
//         Wh_Log(L"Error: Couldn't find instructions for symbol %s", symbol_name);
//         return NULL;
//     }
//     // Can we find the same sequence again in the rest of the haystack?
//     size_t index2 = haystack.find(needle, index1 + 1);
//     if (index2 != std::string_view::npos) {
//         Wh_Log(L"Error: Found multiple matches for %s: at %p and at %p", symbol_name, haystack.begin()+index1, haystack.begin()+index2);
//         // log_hexdump((unsigned char*)haystack.begin() + index1 - 0x50, 6);
//         // Wh_Log(L"----------------");
//         // log_hexdump((unsigned char*)haystack.begin() + index2 - 0x50, 6);
//         return NULL;
//     }
//     return haystack.begin() + index1;
// }

// // get address and size of code section via PE header info  (expect around 200 MB)
// // TODO is this actually correct? (does it include superfluous sections of the DLL?)
// // https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
// // https://learn.microsoft.com/en-us/archive/msdn-magazine/2002/february/inside-windows-win32-portable-executable-file-format-in-detail
// std::string_view getCodeSection(HMODULE chromeModule) {
//     if (chromeModule == NULL) return ""sv;
//     IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*) chromeModule;
//     IMAGE_NT_HEADERS* pe_header = (IMAGE_NT_HEADERS*)(((char*)dos_header) + dos_header->e_lfanew);
//     if (pe_header->FileHeader.Machine != 0x8664 || pe_header->OptionalHeader.Magic != 0x20b) {
//         Wh_Log(L"Mod only implemented for 64-bit windows/chrome - machine was 0x%04x and magic was 0x%04x",
//         pe_header->FileHeader.Machine, pe_header->OptionalHeader.Magic);
//         return ""sv;
//     }
//     return std::string_view{
//         ((char*)dos_header) + pe_header->OptionalHeader.BaseOfCode,
//         pe_header->OptionalHeader.SizeOfCode
//     };
// }

// // Find a function address by scanning for specific instruction patterns.
// // We search the entire code section once per function,
// // to ensure we aren't hooking the wrong location.
// // Better safe than sorry, and it shouldn't cause noticeable delay on startup.
// const char* search_function_instructions(std::string_view code_section, function_search fsearch, LPCWSTR symbol_name) {
//     if (code_section.size()==0) return 0;
//     Wh_Log(L"Searching for function %s", symbol_name);
//     const char* addr = unique_search(code_section, fsearch.search, symbol_name);
//     if (addr == NULL) {
//         Wh_Log(L"Could not find function %s; is the mod up to date?", symbol_name);
//         return NULL;
//     }
//     Wh_Log(L"Instructions were found at address: %p", addr);
//     int offset = fsearch.instr_offset;
//     const char* entry = addr - offset;
//     // verify the prologue is what we expect; otherwise search for it
//     // and verify it is preceded by 0xcc INT3 or 0xc3 RET (or ?? JMP)
//     auto prologue = fsearch.prologue;
//     if (prologue != std::string_view{entry, prologue.size()}) {
//         Wh_Log(L"Prologue not found where expected, searching...");
//         // maybe function length changed due to different compilation
//         auto search_space = std::string_view{entry - 0x40, addr};
//         size_t new_offset = search_space.rfind(prologue);
//         if (new_offset != std::string_view::npos) {
//             entry = (char*)search_space.begin() + new_offset;
//         } else {
//             entry = NULL;
//         }
//     }
//     if (entry) {
//         Wh_Log(L"Found entrypoint for function %s at addr %p", symbol_name, entry);
//         if (entry[-1]!=(char)0xcc && entry[-1]!=(char)0xc3) {
//             Wh_Log(L"Warn: prologue not preceded by INT3 or RET");
//         }
//         return entry;
//     } else {
//         Wh_Log(L"Err: Couldn't locate function entry point for symbol %s", symbol_name);
//         // log_hexdump(addr - 0x40, 0x5);
//         return NULL;
//     }
// }

// #pragma endregion "Code section scan for hooking without symbols"

typedef SkColor CEF_EXPORT (*GetBackgroundColor_t)(void* self, void* browser_settings, int windowless_state);
GetBackgroundColor_t CEF_EXPORT GetBackgroundColor_original;
SkColor CEF_EXPORT GetBackgroundColor_hook(void* self, void* browser_settings, int windowless_state) {
    //SkColor sk_color = GetBackgroundColor_original(self, browser_settings, windowless_state);
    Wh_Log(L"GetBackgroundColor: %d", windowless_state);
    return 0x00000000;
}

/*
CefContext::GetBackgroundColor(CefStructBase<CefBrowserSettingsTraits> const *,cef_state_t)	.text	00000001802A6750	00000091	00000000		R	.	.	.	.	.	.	T	.	.

SkColor CefContext::GetBackgroundColor(
    const CefBrowserSettings* browser_settings,
    cef_state_t windowless_state) const {
  bool is_windowless = windowless_state == STATE_ENABLED
                           ? true
                           : (windowless_state == STATE_DISABLED
                                  ? false
                                  : !!settings_.windowless_rendering_enabled);

  // Default to opaque white if no acceptable color values are found.
  SkColor sk_color = SK_ColorWHITE;

  if (!browser_settings ||
      !GetColor(browser_settings->background_color, is_windowless, &sk_color)) {
    GetColor(settings_.background_color, is_windowless, &sk_color);
  }
  return sk_color;
}

.text:00000001802A6750 ; __int64 __fastcall CefContext::GetBackgroundColor(CefContext *this, const CefStructBase<CefBrowserSettingsTraits> *browser_settings, $A54509D3A8D4A7C7703F367AAB55C308 windowless_state)
.text:00000001802A6750 ?GetBackgroundColor@CefContext@@QEBAIPEBV?$CefStructBase@UCefBrowserSettingsTraits@@@@W4cef_state_t@@@Z proc near
.text:00000001802A6750                                         ; CODE XREF: CefBrowserHostBase::GetBackgroundColor(void)+40↑j
.text:00000001802A6750                                         ; CefBrowserPlatformDelegate::Create(CefBrowserCreateParams const &)+4A↑p ...
.text:00000001802A6750 this = rcx
.text:00000001802A6750 browser_settings = rdx
.text:00000001802A6750 windowless_state = r8d
.text:00000001802A6750                 cmp     windowless_state, 1        41 83 F8 01
.text:00000001802A6754                 jz      short loc_1802A6766        74 10
.text:00000001802A6756                 cmp     windowless_state, 2        41 83 F8 02
.text:00000001802A675A                 jz      short loc_1802A676B        74 0F
.text:00000001802A675C                 cmp     dword ptr [this+7Ch], 0    83 79 7C 00
.text:00000001802A6760                 setnz   r8b                        41 0F 95 C0
.text:00000001802A6764                 jmp     short loc_1802A676E        EB 08
.text:00000001802A6766 ; ---------------------------------------------------------------------------
.text:00000001802A6766
.text:00000001802A6766 loc_1802A6766:                          ; CODE XREF: CefContext::GetBackgroundColor(CefStructBase<CefBrowserSettingsTraits> const *,cef_state_t)+4↑j
.text:00000001802A6766                 mov     r8b, 1                     41 B8 01
.text:00000001802A6769                 jmp     short loc_1802A676E        EB 03
.text:00000001802A676B ; ---------------------------------------------------------------------------
.text:00000001802A676B
.text:00000001802A676B loc_1802A676B:                          ; CODE XREF: CefContext::GetBackgroundColor(CefStructBase<CefBrowserSettingsTraits> const *,cef_state_t)+A↑j
.text:00000001802A676B                 xor     windowless_state, windowless_state  45 31 C0
.text:00000001802A676E
.text:00000001802A676E loc_1802A676E:                          ; CODE XREF: CefContext::GetBackgroundColor(CefStructBase<CefBrowserSettingsTraits> const *,cef_state_t)+14↑j
.text:00000001802A676E                                         ; CefContext::GetBackgroundColor(CefStructBase<CefBrowserSettingsTraits> const *,cef_state_t)+19↑j
.text:00000001802A676E                 test    browser_settings, browser_settings
.text:00000001802A6771                 jz      short loc_1802A67A8
.text:00000001802A6773                 mov     edx, [browser_settings+100h]
.text:00000001802A6779                 cmp     edx, 0FF000000h
.text:00000001802A677F                 setnb   r9b
.text:00000001802A6783                 or      r9b, r8b
.text:00000001802A6786                 mov     r10d, edx
.text:00000001802A6789                 or      r10d, 0FF000000h
.text:00000001802A6790                 xor     eax, eax
.text:00000001802A6792                 cmp     edx, 1000000h
.text:00000001802A6798                 cmovnb  eax, r10d
.text:00000001802A679C                 test    r8b, r8b
.text:00000001802A679F                 cmovz   eax, r10d
.text:00000001802A67A3                 test    r9b, r9b
.text:00000001802A67A6                 jnz     short locret_1802A67E0
.text:00000001802A67A8
.text:00000001802A67A8 loc_1802A67A8:                          ; CODE XREF: CefContext::GetBackgroundColor(CefStructBase<CefBrowserSettingsTraits> const *,cef_state_t)+21↑j
.text:00000001802A67A8                 mov     ecx, [this+17Ch]
.text:00000001802A67AE                 cmp     ecx, 0FF000000h
.text:00000001802A67B4                 setb    dl
.text:00000001802A67B7                 mov     r9d, windowless_state
.text:00000001802A67BA                 not     r9b
.text:00000001802A67BD                 mov     eax, 0FFFFFFFFh
.text:00000001802A67C2                 test    r9b, dl
.text:00000001802A67C5                 jnz     short locret_1802A67E0
.text:00000001802A67C7                 mov     edx, ecx
.text:00000001802A67C9                 or      edx, 0FF000000h
.text:00000001802A67CF                 xor     eax, eax
.text:00000001802A67D1                 cmp     ecx, 1000000h
.text:00000001802A67D7                 cmovnb  eax, edx
.text:00000001802A67DA                 test    r8b, r8b
.text:00000001802A67DD                 cmovz   eax, edx
.text:00000001802A67E0
.text:00000001802A67E0 locret_1802A67E0:                       ; CODE XREF: CefContext::GetBackgroundColor(CefStructBase<CefBrowserSettingsTraits> const *,cef_state_t)+56↑j
.text:00000001802A67E0                                         ; CefContext::GetBackgroundColor(CefStructBase<CefBrowserSettingsTraits> const *,cef_state_t)+75↑j
.text:00000001802A67E0                 retn
.text:00000001802A67E0 ?GetBackgroundColor@CefContext@@QEBAIPEBV?$CefStructBase@UCefBrowserSettingsTraits@@@@W4cef_state_t@@@Z endp
*/

typedef int (*cef_version_info_t)(int entry);

void LoadSettings() {
    cte_settings.showframe = Wh_GetIntSetting(L"showframe");
    cte_settings.showframeonothers = Wh_GetIntSetting(L"showframeonothers");
    cte_settings.showmenu = Wh_GetIntSetting(L"showmenu");
    cte_settings.showcontrols = Wh_GetIntSetting(L"showcontrols");
    cte_settings.transparentcontrols = Wh_GetIntSetting(L"transparentcontrols");
    cte_settings.ignoreminsize = Wh_GetIntSetting(L"ignoreminsize");
    cte_settings.allowuntested = Wh_GetIntSetting(L"allowuntested");
}

int FindOffset(int major, int minor, cte_offset_t offsets[], int offsets_size, BOOL allow_untested = TRUE) {
    int prev_major = offsets[0].ver_major;
    for (int i = 0; i < offsets_size; i++) {
        if (major <= offsets[i].ver_major && major >= prev_major) {
            if (offsets[i].ver_minor == ANY_MINOR ||
                (minor == offsets[i].ver_minor && major == offsets[i].ver_major) // mandate exact major match here
            ) {
                #ifdef _WIN64
                    return offsets[i].offset_x64;
                #else
                    return offsets[i].offset_x86;
                #endif
            }
        }
        prev_major = offsets[i].ver_major;
    }
    if (allow_untested && major >= offsets[offsets_size - 1].ver_major) {
        #ifdef _WIN64
            return offsets[offsets_size - 1].offset_x64;
        #else
            return offsets[offsets_size - 1].offset_x86;
        #endif
    }
    return NULL;
}

// The mod is being initialized, load settings, hook functions, and do other
// initialization stuff if required.
BOOL Wh_ModInit() {
    #ifdef _WIN64
        Wh_Log(L"Init - x86_64");
    #else
        Wh_Log(L"Init - x86");
    #endif

    LoadSettings();

    // Check if this process is auxilliary process by checking if the arguments contain --type=
    LPWSTR args = GetCommandLineW();
    if (wcsstr(args, L"--type=") != NULL) {
        Wh_Log(L"Auxilliary process detected, skipping");
        return FALSE;
    }

    HMODULE cefModule = LoadLibrary(L"libcef.dll");
    if (!cefModule) {
        Wh_Log(L"Failed to load CEF!");
        return FALSE;
    }
    cef_window_create_top_level_t cef_window_create_top_level =
        (cef_window_create_top_level_t)GetProcAddress(cefModule,
                                                "cef_window_create_top_level");
    cef_panel_create_t cef_panel_create =
        (cef_panel_create_t)GetProcAddress(cefModule, "cef_panel_create");
    cef_version_info_t cef_version_info =
        (cef_version_info_t)GetProcAddress(cefModule, "cef_version_info");

    // Get CEF version
    int major = cef_version_info(0);
    int minor = cef_version_info(1);
    Wh_Log(L"CEF v%d.%d.%d.%d (Chromium v%d.%d.%d.%d) Loaded",
        major,
        minor,
        cef_version_info(2),
        cef_version_info(3),
        cef_version_info(4),
        cef_version_info(5),
        cef_version_info(6),
        cef_version_info(7)
    );

    // Check if the app is Spotify
    wchar_t exeName[MAX_PATH];
    GetModuleFileName(NULL, exeName, MAX_PATH);
    BOOL isSpotify = wcsstr(_wcsupr(exeName), L"SPOTIFY.EXE") != NULL;
    if (isSpotify) {
        Wh_Log(L"Spotify detected");
    }

    // Get appropriate offsets for current CEF version
    is_frameless_offset = FindOffset(major, minor, is_frameless_offsets, ARRAYSIZE(is_frameless_offsets));
    Wh_Log(L"is_frameless offset: %#x", is_frameless_offset);
    get_window_handle_offset = FindOffset(major, minor, get_window_handle_offsets, ARRAYSIZE(get_window_handle_offsets), cte_settings.allowuntested);
    Wh_Log(L"get_window_handle offset: %#x", get_window_handle_offset);

    if (isSpotify) {
        add_child_view_offset = FindOffset(major, minor, add_child_view_offsets, ARRAYSIZE(add_child_view_offsets));
        Wh_Log(L"add_child_view offset: %#x", add_child_view_offset);
        set_background_color_offset = FindOffset(major, minor, set_background_color_offsets, ARRAYSIZE(set_background_color_offsets));
        Wh_Log(L"set_background_color offset: %#x", set_background_color_offset);
    }

    if ((is_frameless_offset == NULL || !cte_settings.showframe) &&
        (!isSpotify || add_child_view_offset == NULL || (cte_settings.showmenu && cte_settings.showcontrols)) &&
        !cte_settings.showframeonothers && !cte_settings.ignoreminsize
    ) {
        Wh_Log(L"Nothing to hook, exiting");
        if (is_frameless_offset == NULL) {
            Wh_Log(L"This version of CEF is not supported!");
        }
        return FALSE;
    }

    Wh_SetFunctionHook((void*)cef_window_create_top_level,
                       (void*)cef_window_create_top_level_hook,
                       (void**)&cef_window_create_top_level_original);
    Wh_SetFunctionHook((void*)CreateWindowExW, (void*)CreateWindowExW_hook,
                       (void**)&CreateWindowExW_original);
    if (isSpotify) {
        Wh_SetFunctionHook((void*)cef_panel_create, (void*)cef_panel_create_hook,
                           (void**)&cef_panel_create_original);
        Wh_SetFunctionHook((void*)SetWindowThemeAttribute, (void*)SetWindowThemeAttribute_hook,
                           (void**)&SetWindowThemeAttribute_original);
    }

    // Hook CefContext::GetBackgroundColor
    // std::string_view code_section = getCodeSection(cefModule);
    // const std::string_view GetBackgroundColor_instructions = {
    //     "\x41\x83\xf8\x01"sv
    //     "\x74\x10"sv
    //     "\x41\x83\xf8\x02"sv
    //     "\x74\x0f"sv
    //     "\x83\x79\x7c\x00"sv
    //     "\x41\x0f\x95\xc0"sv
    //     "\xeb\x08"sv
    //     "\x41\xb8\x01"sv
    //     "\xeb\x03"sv
    //     "\x45\x31\xc0"sv;
    // };
    // }
    // if (code_section.size() > 0) {
    //     const char* get_background_color = search_function_instructions(
    //         code_section,
    //         {

    // Hook this to get Aero Glass on Spotify without Chrome runtime
    // Hex patch Spotify.exe as instructed in the main universal mod README to get Aero Glass on Spotify with Chrome runtime
    // Hook CefContext::GetBackgroundColor with static address
    void* GetBackgroundColor_addr = (void*)((char*)cefModule + 0x2A6750);
    Wh_SetFunctionHook(GetBackgroundColor_addr, (void*)GetBackgroundColor_hook, (void**)GetBackgroundColor_original);

    EnumWindows(InitEnumWindowsProc, 1);
    return TRUE;
}

// The mod is being unloaded, free all allocated resources.
void Wh_ModUninit() {
    Wh_Log(L"Uninit");
    EnumWindows(UninitEnumWindowsProc, 1);

    // Restore the original set_background_color functions to prevent crashes
    // (Control colors hooks won't work till the app is restarted)
    for (int i = 0; i < 4; i++) {
        if (cte_controls[i].set_background_color_addr != NULL) {
            *((set_background_color_t*)cte_controls[i].set_background_color_addr) = cte_controls[i].set_background_color_original;
        }
    }
}

// The mod setting were changed, reload them.
void Wh_ModSettingsChanged() {
    BOOL prev_transparentcontrols = cte_settings.transparentcontrols;
    LoadSettings();
    EnumWindows(UpdateEnumWindowsProc, prev_transparentcontrols != cte_settings.transparentcontrols);
}