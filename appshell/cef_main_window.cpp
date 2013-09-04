/*
 * Copyright (c) 2013 Adobe Systems Incorporated. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "cefclient.h"
#include "cef_main_window.h"
#include "cef_registry.h"
#include "client_handler.h"
#include "resource.h"
#include "native_menu_model.h"

// external
extern HINSTANCE	            gInstance;
extern CefRefPtr<ClientHandler> gHandler;

// constants
// TODO: prefix with k instead of g
static const wchar_t		gWindowClassname[] = L"CEFCLIENT";
static const wchar_t		gWindowPostionFolder[] = L"Window Position";

static const wchar_t		gPrefLeft[] = L"Left";
static const wchar_t		gPrefTop[] = L"Top";
static const wchar_t		gPrefWidth[] = L"Width";
static const wchar_t		gPrefHeight[] = L"Height";

static const wchar_t		gPrefRestoreLeft[] = L"Restore Left";
static const wchar_t		gPrefRestoreTop[] = L"Restore Top";
static const wchar_t		gPrefRestoreRight[] = L"Restore Right";
static const wchar_t		gPrefRestoreBottom[]	= L"Restore Bottom";
static const wchar_t		gPrefShowState[] = L"Show State";

static const long			gMinWindowWidth = 390;
static const long			gMinWindowHeight = 200;

static const int            kBorderThickness = 4;
// Globals
wchar_t                     gCefWindowClosingPropName[] = L"CLOSING";


ATOM cef_main_window::RegisterWndClass()
{
	static ATOM classAtom = 0;
	if (classAtom == 0) 
	{
		DWORD menuId = IDC_CEFCLIENT;
		WNDCLASSEX wcex;

		::ZeroMemory (&wcex, sizeof (wcex));
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.lpfnWndProc   = ::DefWindowProc;
		wcex.hInstance     = ::gInstance;
		wcex.hIcon         = ::LoadIcon(gInstance, MAKEINTRESOURCE(IDI_CEFCLIENT));
		wcex.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName  = MAKEINTRESOURCE(menuId);
		wcex.lpszClassName = ::gWindowClassname;
		wcex.hIconSm       = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		classAtom = RegisterClassEx(&wcex);
	}
    return classAtom;
}


cef_main_window::cef_main_window() 
{

}

cef_main_window::~cef_main_window() 
{

}

LPCWSTR cef_main_window::GetBracketsWindowTitleText()
{
    bool intitialized = false;
    static wchar_t szTitle[100] = L"";

    if (!intitialized) {
        intitialized  = (::LoadString(::gInstance, IDS_APP_TITLE, szTitle, _countof(szTitle) - 1) > 0);
    }
    return szTitle;
}



BOOL cef_main_window::Create() 
{
	RegisterWndClass();

	int left   = CW_USEDEFAULT;
	int top    = CW_USEDEFAULT;
	int width  = CW_USEDEFAULT;
	int height = CW_USEDEFAULT;
	int showCmd = SW_SHOW;

	RestoreWindowRect(left, top, width, height, showCmd);

	DWORD styles =  WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_EX_COMPOSITED | WS_CAPTION | WS_SYSMENU  | WS_THICKFRAME  |WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	if (showCmd == SW_MAXIMIZE)
	  styles |= WS_MAXIMIZE;

	if (!cef_window::Create(::gWindowClassname, GetBracketsWindowTitleText(),
								styles, left, top, width, height))
	{
		return FALSE;
	}

	RestoreWindowPlacement(showCmd);
	UpdateWindow();

    return TRUE;
}

void cef_main_window::PostNcDestory()
{
	delete this;
}

void cef_main_window::GetCefBrowserRect(RECT& rect)
{
    GetClientRect(&rect);
}


BOOL cef_main_window::HandleCreate() 
{
	// Create the single static handler class instance
    gHandler = new ClientHandler();
    gHandler->SetMainHwnd(mWnd);

    RECT rect;

    GetCefBrowserRect(rect);

    CefWindowInfo info;
    CefBrowserSettings settings;

    settings.web_security = STATE_DISABLED;

    // Initialize window info to the defaults for a child window
    info.SetAsChild(mWnd, rect);

    // Creat the new child browser window
    CefBrowserHost::CreateBrowser(info,
        static_cast<CefRefPtr<CefClient> >(gHandler),
        ::AppGetInitialURL(), settings);

	return TRUE;
}

CefWindowHandle cef_main_window::SafeGetCefBrowserHwnd()
{
	if (gHandler.get() && gHandler->GetBrowser())
		return gHandler->GetBrowser()->GetHost()->GetWindowHandle();
	return NULL;
}


BOOL cef_main_window::HandleEraseBackground()
{
	return (SafeGetCefBrowserHwnd() != NULL);
}

BOOL cef_main_window::HandleSetFocus(HWND hLosingFocus)
{
    // Pass focus to the browser window
    CefWindowHandle hwnd = SafeGetCefBrowserHwnd();
    if (hwnd) 
	{
		::PostMessage(hwnd, WM_SETFOCUS, (WPARAM)hLosingFocus, NULL);
		return TRUE;
	}
	return FALSE;
}

BOOL cef_main_window::HandlePaint()
{
    // avoid painting
    PAINTSTRUCT ps;
	BeginPaint(&ps);
	EndPaint(&ps);
    return TRUE;
}


BOOL cef_main_window::HandleGetMinMaxInfo(LPMINMAXINFO mmi)
{
	mmi->ptMinTrackSize.x = ::gMinWindowWidth;
	mmi->ptMinTrackSize.y = ::gMinWindowHeight;




    return TRUE;
}

BOOL cef_main_window::HandleDestroy()
{
	::PostQuitMessage(0);
	return TRUE;
}

BOOL cef_main_window::HandleClose()
{
    SaveWindowRect();

	CefWindowHandle hwnd = SafeGetCefBrowserHwnd();
    if (hwnd)
    {
        BOOL closing = (BOOL)::GetProp(hwnd, ::gCefWindowClosingPropName);
        if (closing) 
		{
			::RemoveProp(hwnd, ::gCefWindowClosingPropName);
		} 
		else 
		{
			gHandler->QuittingApp(true);
			gHandler->DispatchCloseToNextBrowser();
    		return TRUE;
		}
    } 
		
	return FALSE;
}

BOOL cef_main_window::HandleExitCommand()
{
	if (gHandler.get()) 
    {
		gHandler->QuittingApp(true);
		gHandler->DispatchCloseToNextBrowser();
	}
    else 
    {
		DestroyWindow();
	}
	return TRUE;
}


BOOL cef_main_window::HandleSize(BOOL bMinimize)
{
	// Minimizing the window to 0x0 which causes our layout to go all
	// screwy, so we just ignore it.
	CefWindowHandle hwnd = SafeGetCefBrowserHwnd();
	if (hwnd && !bMinimize) 
	{
		RECT rect;
		GetClientRect(&rect);

		HDWP hdwp = ::BeginDeferWindowPos(1);
        hdwp = ::DeferWindowPos(hdwp, hwnd, NULL, rect.left, rect.top, ::RectWidth(rect), ::RectHeight(rect), SWP_NOZORDER);
		::EndDeferWindowPos(hdwp);
	}

	return FALSE;
}

BOOL cef_main_window::HandleInitMenuPopup(HMENU hMenuPopup)
{
    int count = ::GetMenuItemCount(hMenuPopup);
    void* menuParent = ::getMenuParent(gHandler->GetBrowser());
    for (int i = 0; i < count; i++) {
        UINT id = ::GetMenuItemID(hMenuPopup, i);

        bool enabled = NativeMenuModel::getInstance(menuParent).isMenuItemEnabled(id);
        UINT flagEnabled = enabled ? MF_ENABLED : (MF_GRAYED | MF_DISABLED);
        EnableMenuItem(hMenuPopup, id,  flagEnabled | MF_BYCOMMAND);

        bool checked = NativeMenuModel::getInstance(menuParent).isMenuItemChecked(id);
        UINT flagChecked = checked ? MF_CHECKED : MF_UNCHECKED;
        ::CheckMenuItem(hMenuPopup, id, flagChecked | MF_BYCOMMAND);
    }
	return TRUE;
}


BOOL cef_main_window::HandleCommand(UINT commandId)
{
	switch (commandId) 
	{
	case IDM_EXIT:
		HandleExitCommand();
		return TRUE;
	default:
        ExtensionString commandString = NativeMenuModel::getInstance(::getMenuParent(gHandler->GetBrowser())).getCommandId(commandId);
        if (commandString.size() > 0) 
		{
            gHandler->SendJSCommand(gHandler->GetBrowser(), commandString);
			return TRUE;
        }
	}

	return FALSE;
}

void cef_main_window::SaveWindowRect()
{
	WINDOWPLACEMENT wp;
	::ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	 
	if (GetWindowPlacement(&wp))
	{
		// Only save window positions for "restored" and "maximized" states.
		// If window is closed while "minimized", we don't want it to open minimized
		// for next session, so don't update registry so it opens in previous state.
		if (wp.showCmd == SW_SHOWNORMAL || wp.showCmd == SW_SHOW || wp.showCmd == SW_MAXIMIZE)
		{
			RECT rect;
			if (GetWindowRect(&rect))
			{
				::WriteRegistryInt(gWindowPostionFolder, gPrefLeft,   rect.left);
				::WriteRegistryInt(gWindowPostionFolder, gPrefTop,    rect.top);
                ::WriteRegistryInt(gWindowPostionFolder, gPrefWidth,  ::RectWidth(rect));
				::WriteRegistryInt(gWindowPostionFolder, gPrefHeight, ::RectHeight(rect));
			}

			if (wp.showCmd == SW_MAXIMIZE)
			{
				// When window is maximized, we also store the "restore" size
				::WriteRegistryInt(gWindowPostionFolder, gPrefRestoreLeft,   wp.rcNormalPosition.left);
				::WriteRegistryInt(gWindowPostionFolder, gPrefRestoreTop,    wp.rcNormalPosition.top);
				::WriteRegistryInt(gWindowPostionFolder, gPrefRestoreRight,  wp.rcNormalPosition.right);
				::WriteRegistryInt(gWindowPostionFolder, gPrefRestoreBottom, wp.rcNormalPosition.bottom);
			}

			// Maximize is the only special case we handle
			::WriteRegistryInt(gWindowPostionFolder, gPrefShowState,
							 (wp.showCmd == SW_MAXIMIZE) ? SW_MAXIMIZE : SW_SHOW);
		}
	}
}


void cef_main_window::RestoreWindowRect(int& left, int& top, int& width, int& height, int& showCmd)
{
	::GetRegistryInt(gWindowPostionFolder, gPrefLeft,  			    NULL, left);
	::GetRegistryInt(gWindowPostionFolder, gPrefTop,   			    NULL, top);
	::GetRegistryInt(gWindowPostionFolder, gPrefWidth, 			    NULL, width);
	::GetRegistryInt(gWindowPostionFolder, gPrefHeight,			    NULL, height);
	::GetRegistryInt(gWindowPostionFolder, gWindowPostionFolder,	NULL, showCmd);
}

void cef_main_window::RestoreWindowPlacement(int showCmd)
{
	if (showCmd == SW_MAXIMIZE)
	{
		WINDOWPLACEMENT wp;
		::ZeroMemory(&wp, sizeof (wp));
		wp.length = sizeof(WINDOWPLACEMENT);

		wp.flags	= 0;
		wp.showCmd	= SW_MAXIMIZE;
		wp.ptMinPosition.x	= -1;
		wp.ptMinPosition.y	= -1;
		wp.ptMaxPosition.x	= -1;
		wp.ptMaxPosition.y	= -1;

		wp.rcNormalPosition.left	= CW_USEDEFAULT;
		wp.rcNormalPosition.top		= CW_USEDEFAULT;
		wp.rcNormalPosition.right	= CW_USEDEFAULT;
		wp.rcNormalPosition.bottom	= CW_USEDEFAULT;

		GetRegistryInt(gWindowPostionFolder, gPrefRestoreLeft,	    NULL, (int&)wp.rcNormalPosition.left);
		GetRegistryInt(gWindowPostionFolder, gPrefRestoreTop,		NULL, (int&)wp.rcNormalPosition.top);
		GetRegistryInt(gWindowPostionFolder, gPrefRestoreRight,	    NULL, (int&)wp.rcNormalPosition.right);
		GetRegistryInt(gWindowPostionFolder, gPrefRestoreBottom,	NULL, (int&)wp.rcNormalPosition.bottom);

		// This returns FALSE on failure, but not sure what we could in that case
		SetWindowPlacement(&wp);
	}

    ShowWindow(showCmd);
}

BOOL cef_main_window::HandleCopyData(HWND, PCOPYDATASTRUCT lpCopyData) 
{
	if ((lpCopyData) && (lpCopyData->dwData == ID_WM_COPYDATA_SENDOPENFILECOMMAND) && (lpCopyData->cbData > 0)) {
		// another Brackets instance requests that we open the given filename
		std::wstring wstrFilename = (LPCWSTR)lpCopyData->lpData;

        // Windows Explorer might enclose the filename in double-quotes.  We need to strip these off.
		if ((wstrFilename.front() == '\"') && wstrFilename.back() == '\"')
			wstrFilename = wstrFilename.substr(1, wstrFilename.length() - 2);

		gHandler->SendOpenFileCommand(gHandler->GetBrowser(), CefString(wstrFilename.c_str()));
        return true;
	}

    return false;
}

// EnumWindowsProc callback function
//  - searches for an already running Brackets application window
BOOL CALLBACK cef_main_window::FindSuitableBracketsInstanceHelper(HWND hwnd, LPARAM lParam)
{
	if (lParam == NULL)
        return FALSE;

	// check for the Brackets application window by class name and title
	WCHAR cName[MAX_PATH+1] = {0}, cTitle[MAX_PATH+1] = {0};
	::GetClassName(hwnd, cName, MAX_PATH);
	::GetWindowText(hwnd, cTitle, MAX_PATH);

	if ((wcscmp(cName, ::gWindowClassname) == 0) && (wcsstr(cTitle, GetBracketsWindowTitleText()) != 0)) {
		// found an already running instance of Brackets.  Now, check that that window
		//   isn't currently disabled (eg. modal dialog).  If it is keep searching.
		if ((::GetWindowLong(hwnd, GWL_STYLE) & WS_DISABLED) == 0) {
			//return the window handle and stop searching
			*(HWND*)lParam = hwnd;
			return FALSE;
		}
	}

	return TRUE;	// otherwise, continue searching
}

HWND cef_main_window::FindFirstTopLevelInstance()
{
    HWND hFirstInstanceWnd = NULL;
    ::EnumWindows(FindSuitableBracketsInstanceHelper, (LPARAM)&hFirstInstanceWnd);
    return hFirstInstanceWnd;
}


LRESULT cef_main_window::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_CREATE:
		if (HandleCreate())
			return 0L;
		break;
	case WM_ERASEBKGND:
		if (HandleEraseBackground())
			return 1L;
		break;
	case WM_SETFOCUS:
		if (HandleSetFocus((HWND)wParam))
			return 0L;
		break;
	case WM_PAINT:
		if (HandlePaint())
			return 0L;
		break;
	case WM_GETMINMAXINFO:
		if (HandleGetMinMaxInfo((LPMINMAXINFO) lParam))
			return 0L;
		break;
	case WM_DESTROY:
		if (HandleDestroy())
			return 0L;
		break;
	case WM_CLOSE:
		if (HandleClose())
			return 0L;
		break;
	case WM_SIZE:
		if (HandleSize(wParam == SIZE_MINIMIZED))
			return 0L;
		break;
	case WM_INITMENUPOPUP:
		if (HandleInitMenuPopup((HMENU)wParam))
			return 0L;
		break;
	case WM_COMMAND:
		if (HandleCommand(LOWORD(wParam)))
			return 0L;
		break;
    case WM_COPYDATA:
        if (HandleCopyData((HWND)wParam, (PCOPYDATASTRUCT)lParam))
            return 0L;
        break;
    }

    LRESULT lr = cef_window::WindowProc(message, wParam, lParam);

    return lr;
}
