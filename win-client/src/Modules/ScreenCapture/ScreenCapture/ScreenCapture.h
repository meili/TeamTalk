
/*******************************************************************************
 *  @file      ScreenCapture.h 2015\5\5 15:54:37 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef SCREENCAPTURE_9A94982E_1F10_418A_8424_4E211A84E305_H__
#define SCREENCAPTURE_9A94982E_1F10_418A_8424_4E211A84E305_H__

#include "Common.h"
#include <vector>
/******************************************************************************/

/**
 * The class <code>ScreenCapture</code> 
 *
 */

class ScreenCaptureCallback
{
public:
	virtual void onScreenCaptureFinish(__in std::wstring resultPicPath) = 0;
};

enum ScreenCaptureHotykeyId
{
    SCHK_MIN = 0,
    SCHK_START_CAPTURE,
    SCHK_ESCAPE,

    SCHK_MAX
};

class ScreenCapture
{
public:
	static ScreenCapture *getInstance();

public:
	ScreenCapture();
	~ScreenCapture();
	BOOL initCapture(__in HWND hWnd);
	BOOL startCapture(__in std::wstring strSavePath, __in ScreenCaptureCallback *captureCallback = NULL,
		__in BOOL bMinimizeWindow = FALSE);
    ScreenCaptureHotykeyId shouldHandle(__in LPARAM lParam);

	void finishCapture();
	void cancelCapture();

private:
    BOOL createMsgWindow();
	BOOL installMsgHook(BOOL bInstall);
	HHOOK getMouseHook(){ return m_hMouseHook; }

private:
	static LRESULT CALLBACK MouseProc(
		_In_  int nCode,
		_In_  WPARAM wParam,
		_In_  LPARAM lParam
		);

    static LRESULT CALLBACK MsgWindowProc(
        _In_ HWND   hwnd,
        _In_ UINT   uMsg,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam
        );

private:
	std::wstring m_strSavePath;
	ScreenCaptureCallback *m_callBack;

private:
	HHOOK m_hMouseHook;
	HWND m_hMinimizedWindow;

private:
	HWND m_hRegisterHotkeyWnd;
	int m_iHotkeyId;

private:
    BOOL m_bInCapture;
    CRITICAL_SECTION m_sInCaptureLock;
};

/******************************************************************************/
#endif// SCREENCAPTURE_9a94982e-1f10-418a-8424-4e211a84e305_H__