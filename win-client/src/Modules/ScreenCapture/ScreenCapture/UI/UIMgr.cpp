
/******************************************************************************* 
 *  @file      UIMgr.cpp 2015\5\5 16:03:37 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "UIMgr.h"
#include "BkgroundUI.h"
#include "EditToolbarUI.h"
#include "../ScreenSnapshot.h"

/******************************************************************************/

UIScreenCaptureMgr::UIScreenCaptureMgr()
{

}


void UIScreenCaptureMgr::forceForgroundWindow(__in HWND hWnd)
{
    HWND hForegroundWnd = ::GetForegroundWindow();
    DWORD dwPid = GetWindowThreadProcessId(hForegroundWnd, NULL);
    if (!AttachThreadInput(dwPid, GetCurrentThreadId(), TRUE)
        || !::SetForegroundWindow(hWnd) 
        || !::BringWindowToTop(hWnd))
    {
        return;
    }

    SwitchToThisWindow(hWnd, TRUE);
    AttachThreadInput(dwPid, GetCurrentThreadId(), FALSE);
}

BOOL UIScreenCaptureMgr::createWindows()
{
	m_hBkgUI = BkgroundUI::Instance()->createWindow();

	wchar_t szImg[MAX_PATH] = {0};
	GetModuleFileName(NULL, szImg, MAX_PATH);
	PathRemoveFileSpec(szImg);
	PathRemoveFileSpec(szImg);

	std::wstring strBkgPic = std::wstring(szImg) + L"\\gui\\ScreenCapture\\sc_toolbar_normal.png";
	std::wstring strHoverPic = std::wstring(szImg) + L"\\gui\\ScreenCapture\\sc_toolbar_hover.png";
	std::wstring strSelPic = std::wstring(szImg) + L"\\gui\\ScreenCapture\\sc_toolbar_select.png";
	

	EditToolbarInfo toolBarInfo = {
		0, 0, 193, 37,
		strBkgPic,
		strHoverPic,
		strSelPic,
		{
			{ 9, 5, 35, 31 },
			{ 43, 5, 69, 31 },
			{ 85, 5, 112, 31 },
			{ 119, 5, 185, 31 }
		}
	};

	m_hEditToolBarUI = EditToolbarUI::Instance()->createWindow(toolBarInfo, m_hBkgUI);
    SetWindowPos(m_hBkgUI, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    forceForgroundWindow(m_hBkgUI);
	ShowWindow(m_hBkgUI, SW_SHOW);

	return TRUE;
}

void UIScreenCaptureMgr::changeBkCursor(__in BOOL bMoveCursor)
{
	if (bMoveCursor)
	{
		SetClassLong(m_hBkgUI, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_SIZEALL));
	}
	else
	{
		SetClassLong(m_hBkgUI, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
	}
}
BOOL UIScreenCaptureMgr::closeWindows()
{
	BkgroundUI::Instance()->closeWindow();
	EditToolbarUI::Instance()->closeWindow();

	m_hBkgUI = NULL;
	m_hEditToolBarUI = NULL;

	return TRUE;
}

void UIScreenCaptureMgr::postBkgMessage(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam)
{
	::PostMessage(m_hBkgUI, uMsg, wParam, lParam);
}

void UIScreenCaptureMgr::sendBkgMessage(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam)
{
	::SendMessage(m_hBkgUI, uMsg, wParam, lParam);
}

void UIScreenCaptureMgr::postEditToolbarMessage(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam)
{
	::PostMessage(m_hEditToolBarUI, uMsg, wParam, lParam);
}

void UIScreenCaptureMgr::sendEditToolbarMessage(__in UINT uMsg, __in WPARAM wParam, __in LPARAM lParam)
{
	::SendMessage(m_hEditToolBarUI, uMsg, wParam, lParam);
}

/******************************************************************************/