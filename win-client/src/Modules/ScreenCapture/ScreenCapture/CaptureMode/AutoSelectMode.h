
/*******************************************************************************
 *  @file      AutoSelectMode.h 2015\5\5 16:15:10 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef AUTOSELECTMODE_170FD8AC_FD77_4159_8DE6_3974793BBF0F_H__
#define AUTOSELECTMODE_170FD8AC_FD77_4159_8DE6_3974793BBF0F_H__

#include "ModeComm.h"
#include <vector>
/******************************************************************************/

/**
 * The class <code>AutoSelectMode</code> 
 *
 */



class ScreenCaptureWndInfo
{
private:
	HWND m_hWnd;
	RECT m_rcWnd;

public:
	int getX()const { return m_rcWnd.left; }
	int getY()const { return m_rcWnd.top; }
	HWND getHwnd()const { return m_hWnd; }
	const RECT *getRect()const { return &m_rcWnd; }

public:
	ScreenCaptureWndInfo(){}
	ScreenCaptureWndInfo(__in HWND hWnd, __in RECT &rc)
	{
		m_hWnd = hWnd;
		memcpy(&m_rcWnd, &rc, sizeof(rc));
	}
	ScreenCaptureWndInfo(__in int x, int y)
	{
		m_hWnd = 0;
		m_rcWnd.left = x;
		m_rcWnd.top = y;
		m_rcWnd.right = 0;
		m_rcWnd.bottom = 0;
	}
	bool operator==(const ScreenCaptureWndInfo &wndInfo) const
	{
		POINT pt = { wndInfo.getX(), wndInfo.getY() };
		return !!PtInRect(&m_rcWnd, pt);
	}
};

class AutoSelectMode :public Singleton<AutoSelectMode>, public IModeMsgHandler
{
public:
	AutoSelectMode();
	BOOL snapshotWindows();
	void clear();

public:
	virtual void onLButtonDown(__in int x, __in int y);
	virtual void onLButtonUp(__in int x, __in int y);
	virtual void onLButtonDBClick(__in int x, __in int y);
	virtual void onMouseMove(__in int x, __in int y);

private:
	BOOL adjustRectInScreen(__in RECT &rc);
	ScreenCaptureWndInfo* windowFromPoint(__in int x, __in int y);

private:
	HWND m_hPrevMouseWindow;
	std::vector<ScreenCaptureWndInfo> m_lsWndList;

private:
	int m_iPrevMouseXPos;
	int m_iPrevMouseYPos;
};

/******************************************************************************/
#endif// AUTOSELECTMODE_170fd8ac-fd77-4159-8de6-3974793bbf0f_H__