
/******************************************************************************* 
 *  @file      ScreenCapture.cpp 2015\5\5 15:55:44 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "ScreenCapture.h"
#include "ScreenSnapshot.h"
#include "UI/UIMgr.h"
#include "CaptureMode/CaptureMode.h"
#include "shlwapi.h"

#pragma comment(lib, "shlwapi.lib")

/******************************************************************************/


ScreenCapture::ScreenCapture():
m_hMouseHook(NULL),
m_callBack(NULL),
m_hMinimizedWindow(NULL),
m_hRegisterHotkeyWnd(NULL),
m_iHotkeyId(0),
m_bInCapture(FALSE)
{
    InitializeCriticalSection(&m_sInCaptureLock);
}

ScreenCapture::~ScreenCapture()
{
	if (m_hRegisterHotkeyWnd && m_iHotkeyId)
	{
		UnregisterHotKey(m_hRegisterHotkeyWnd, m_iHotkeyId);
        GlobalDeleteAtom(m_iHotkeyId);
	}

    m_hRegisterHotkeyWnd = NULL;
    m_iHotkeyId = 0;

    ScreenCommon::postNotifyMessage(WM_CLOSE, 0, 0);
}

ScreenCapture *ScreenCapture::getInstance()
{
	static ScreenCapture obj;
	return &obj;
}

ScreenCaptureHotykeyId ScreenCapture::shouldHandle(__in LPARAM lParam)
{
	if (0x51 == HIWORD(lParam) && (MOD_CONTROL | MOD_SHIFT) == LOWORD(lParam))
	{
		//ctrl + shift + A
		return SCHK_START_CAPTURE;
	}

	return SCHK_MAX;
}

BOOL ScreenCapture::createMsgWindow()
{
    wchar_t szWndName[MAX_PATH] = {0};
    ScreenCommon::generateMsgWindowName(szWndName, MAX_PATH);

    WNDCLASS wndCls = {0};
    wndCls.hInstance = GetModuleHandle(NULL);
    wndCls.lpfnWndProc = MsgWindowProc;
    wndCls.lpszClassName = szWndName;
    RegisterClass(&wndCls);

    return (NULL != CreateWindow(szWndName, szWndName,
        0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL));
}

BOOL ScreenCapture::initCapture(__in HWND hWnd)
{
	//register hot key
	const std::wstring screenCaptureHotkeyName = L"_SCREEN_CAPTURE_HOTKEY";
    int iHotkeyId = (int)GlobalAddAtom(screenCaptureHotkeyName.c_str());

    if (!RegisterHotKey(hWnd, iHotkeyId, MOD_CONTROL | MOD_SHIFT, 0x51)) //ctrl + shift + Q
    {
        GlobalDeleteAtom(iHotkeyId);
    }

	m_iHotkeyId = iHotkeyId;
	m_hRegisterHotkeyWnd = hWnd;

    return createMsgWindow();
}

BOOL ScreenCapture::startCapture(__in std::wstring strSavePath, __in ScreenCaptureCallback *captureCallback/* = NULL*/, __in BOOL bMinimizeWindow/* = FALSE*/)
{
    EnterCriticalSection(&m_sInCaptureLock);
    if (m_bInCapture)
    {
        LeaveCriticalSection(&m_sInCaptureLock);
        return TRUE;
    }
    m_bInCapture = TRUE;
    LeaveCriticalSection(&m_sInCaptureLock);

	m_strSavePath = strSavePath;
	m_callBack = captureCallback;

	m_hMinimizedWindow = NULL;
	if (bMinimizeWindow)
	{
		POINT pt = { 0 };
		GetCursorPos(&pt);
		HWND hWnd = WindowFromPoint(pt);
		if (IsWindow(hWnd))
		{
			m_hMinimizedWindow = hWnd;
			ShowWindow(m_hMinimizedWindow, SW_HIDE);
		}
	}

	BOOL bStartResult = FALSE;
	do 
	{
		if (!ScreenSnapshot::Instance()->snapshotScreen())
		{
			break;
		}

		if (!CaptureModeMgr::Instance()->snapshotWindows())
		{
			break;
		}

		if (!UIScreenCaptureMgr::Instance()->createWindows())
		{
			break;
		}

		CaptureModeMgr::Instance()->changeMode(CM_AUTO_SELECT);
		if (!installMsgHook(TRUE))
		{
			break;
		}
		bStartResult = TRUE;
	} while (false);

	if (!bStartResult)
	{
		cancelCapture();
		return FALSE;
	}

	POINT pt = {0};
	GetCursorPos(&pt);
	mouse_event(MOUSEEVENTF_MOVE, 1, 1, 0, 0);
	return TRUE;
}

void ScreenCapture::cancelCapture()
{
    EnterCriticalSection(&m_sInCaptureLock);
    m_bInCapture = FALSE;
    LeaveCriticalSection(&m_sInCaptureLock);

	installMsgHook(FALSE);
	UIScreenCaptureMgr::Instance()->closeWindows();
	CaptureModeMgr::Instance()->clear();

	if (m_hMinimizedWindow)
	{
		ShowWindow(m_hMinimizedWindow, SW_SHOW);
		m_hMinimizedWindow = NULL;
	}
}

void ScreenCapture::finishCapture()
{
	RECT rcSelect = {0};
	UIScreenCaptureMgr::Instance()->sendBkgMessage(WM_SNAPSHOT_TEST_SELECT_RECT, (WPARAM)&rcSelect, 0);

    rcSelect.left += 2;
    rcSelect.top += 2;
    rcSelect.right -= 2;
    rcSelect.bottom -= 2;
	if (!ScreenCommon::isRectEmpty(rcSelect))
	{
		ScreenSnapshot::Instance()->saveRect(rcSelect, m_strSavePath);
	}
	
	cancelCapture();
	if (m_callBack) m_callBack->onScreenCaptureFinish(m_strSavePath);
}

LRESULT ScreenCapture::MsgWindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    if (WM_SNAPSHOT_MODE_CHANGE == uMsg)
    {
        CaptureModeMgr::Instance()->changeMode(CM_MANAL_EDIT);
    }
    else if (WM_SNAPSHOT_FINISH_CAPTURE == uMsg)
    {
        ScreenCapture::getInstance()->finishCapture();
    }
    else if (WM_SNAPSHOT_CANCEL_CPATURE == uMsg)
    {
        ScreenCapture::getInstance()->cancelCapture();
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
LRESULT ScreenCapture::MouseProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	PMOUSEHOOKSTRUCT pHookInfo = (PMOUSEHOOKSTRUCT)lParam;
	int xPos = pHookInfo->pt.x;
	int yPos = pHookInfo->pt.y;

	LRESULT lResHandled = CallNextHookEx(ScreenCapture::getInstance()->getMouseHook(), nCode, wParam, lParam);
	if (WM_LBUTTONDBLCLK == wParam )
	{
        ScreenCommon::postNotifyMessage(WM_SNAPSHOT_FINISH_CAPTURE, 0, 0);
	}
    else if (WM_RBUTTONDBLCLK == wParam)
    {
        ScreenCommon::postNotifyMessage(WM_SNAPSHOT_CANCEL_CPATURE, 0, 0);
    }
	else if (WM_LBUTTONDOWN == wParam)
	{
        if (CM_AUTO_SELECT == CaptureModeMgr::Instance()->getMode())
        {
            CaptureModeMgr::Instance()->changeMode(CM_MANAL_SELECT);
        }
	}

	CaptureModeMgr::Instance()->handleMouseMsg(wParam, xPos, yPos);
	return lResHandled;
}

BOOL ScreenCapture::installMsgHook(BOOL bInstall)
{
	BOOL result = FALSE;
	if (bInstall)
	{
		if (!m_hMouseHook)
		{
			m_hMouseHook = SetWindowsHookEx(WH_MOUSE, MouseProc, NULL, GetCurrentThreadId());
			result = (NULL != m_hMouseHook);
		}
	}
	else
	{
		UnhookWindowsHookEx(m_hMouseHook);
		m_hMouseHook = NULL;
		result = TRUE;
	}

	return result;
}

/******************************************************************************/