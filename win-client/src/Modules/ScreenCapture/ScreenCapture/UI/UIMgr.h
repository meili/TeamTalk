
/*******************************************************************************
 *  @file      UIMgr.h 2015\5\5 16:02:35 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef UIMGR_937DCEBA_B9CD_4C3E_8101_8014D170D4C4_H__
#define UIMGR_937DCEBA_B9CD_4C3E_8101_8014D170D4C4_H__

#include "../Common.h"
/******************************************************************************/

/**
 * The class <code>UIMgr</code> 
 *
 */

class UIScreenCaptureMgr:public Singleton<UIScreenCaptureMgr>
{
public:
	UIScreenCaptureMgr();
	BOOL createWindows();
	BOOL closeWindows();

public:
	void postBkgMessage(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam);
	void sendBkgMessage(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam);

	void postEditToolbarMessage(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam);
	void sendEditToolbarMessage(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam);


public:
	void changeBkCursor(__in BOOL bMoveCursor);

private:
    void forceForgroundWindow(__in HWND hWnd);

private:
	HWND m_hBkgUI;
	HWND m_hEditToolBarUI;
};

/******************************************************************************/
#endif// UIMGR_937dceba-b9cd-4c3e-8101-8014d170d4c4_H__