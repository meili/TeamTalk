/******************************************************************************* 
 *  @file      AutoSelectMode.cpp 2015\5\5 16:16:47 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "AutoSelectMode.h"
#include "../UI/UIMgr.h"

/******************************************************************************/

AutoSelectMode::AutoSelectMode():
m_hPrevMouseWindow(NULL),
m_iPrevMouseXPos(-1),
m_iPrevMouseYPos(-1)
{
	m_lsWndList.clear();
}

void AutoSelectMode::onLButtonDown(__in int x, __in int y)
{
	//ignore
}

void AutoSelectMode::onLButtonUp(__in int x, __in int y)
{
	//ignore
}

void AutoSelectMode::onLButtonDBClick(__in int x, __in int y)
{
	//ignore
}

void AutoSelectMode::onMouseMove(__in int x, __in int y)
{
	if (x == m_iPrevMouseXPos && y == m_iPrevMouseYPos)
	{
		return;
	}
	m_iPrevMouseXPos = x;
	m_iPrevMouseYPos = y;

	UIScreenCaptureMgr::Instance()->changeBkCursor(FALSE);

	if (ScreenCommon::isLButtonDown())
	{
		return;
	}

	ScreenCaptureWndInfo *wndInfo = windowFromPoint(x, y);
	if (wndInfo->getHwnd() != m_hPrevMouseWindow)
	{
		m_hPrevMouseWindow = wndInfo->getHwnd();
        UIScreenCaptureMgr::Instance()->sendBkgMessage(WM_SNAPSHOT_MOUSE_WINDOW_CHANGE, 0, (LPARAM)wndInfo->getRect());
	}
}

BOOL AutoSelectMode::snapshotWindows()
{
	for (HWND hWnd = GetTopWindow(NULL); NULL != hWnd; hWnd = GetWindow(hWnd, GW_HWNDNEXT))
	{
		if (!IsWindow(hWnd)
			|| !IsWindowVisible(hWnd)
			|| IsIconic(hWnd))
		{
			continue;
		}

		RECT rcWnd = { 0 };
		GetWindowRect(hWnd, &rcWnd);
		adjustRectInScreen(rcWnd);
		if (ScreenCommon::isRectEmpty(rcWnd))
		{
			continue;
		}

		wchar_t szTxt[MAX_PATH] = { 0 };
		GetWindowText(hWnd, szTxt, MAX_PATH);
		if (wcslen(szTxt) <= 0)
		{
			continue;
		}

		//combine the rect with the screen rect
		m_lsWndList.push_back(ScreenCaptureWndInfo(hWnd, rcWnd));
	}

	return m_lsWndList.size() > 0;
}

BOOL AutoSelectMode::adjustRectInScreen(__in RECT &rc)
{
	RECT rcScreen = { 0 };
	GetWindowRect(GetDesktopWindow(), &rcScreen);

	if (rc.top < rcScreen.top) rc.top = rcScreen.top;
	if (rc.left < rcScreen.left) rc.left = rcScreen.left;
	if (rc.right > rcScreen.right) rc.right = rcScreen.right;
	if (rc.bottom > rcScreen.bottom) rc.bottom = rcScreen.bottom;

	return TRUE;
}

ScreenCaptureWndInfo* AutoSelectMode::windowFromPoint(__in int x, __in int y)
{
	std::vector<ScreenCaptureWndInfo>::iterator itr = std::find(m_lsWndList.begin(), m_lsWndList.end(), ScreenCaptureWndInfo(x, y));
	if (itr != m_lsWndList.end())
	{
		return &(*itr);
	}

	return NULL;
}

void AutoSelectMode::clear()
{
	m_hPrevMouseWindow = NULL;
	m_lsWndList.clear();

	m_iPrevMouseXPos = -1;
	m_iPrevMouseYPos = -1;
}



/******************************************************************************/