
/******************************************************************************* 
 *  @file      EditToolbar.cpp 2015\5\5 16:05:04 $
 *  @author    南松<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "EditToolbarUI.h"
#include "../ScreenSnapshot.h"
#include "../../3rdParty/src/cxImage/cxImage/ximage.h"

/******************************************************************************/
IMP_DIALOG_PROC(EditToolbarUI)

EditToolbarUI::EditToolbarUI()
:m_iSelectIdx(-1),
m_iMouseMoveIdx(-1)
{
	
}

void EditToolbarUI::InitMsgMap()
{
	CDlgBase::InitMsgMap();
	RegisterMsg(WM_PAINT, (MSG_HANDLER)&EditToolbarUI::onPaint);
	RegisterMsg(WM_LBUTTONDOWN, (MSG_HANDLER)&EditToolbarUI::onLButtonDown);
    RegisterMsg(WM_KEYDOWN, (MSG_HANDLER)&EditToolbarUI::onKeyDown);
	RegisterMsg(WM_MOUSEMOVE, (MSG_HANDLER)&EditToolbarUI::onMouseMove);
	RegisterMsg(WM_SNAPSHOT_ADJUST_TOOL_BAR_POSITION, (MSG_HANDLER)&EditToolbarUI::onAdjustToolbarPosition);
	RegisterMsg(WM_SNAPSHOT_TEST_SELECT_IDX, (MSG_HANDLER)&EditToolbarUI::onTestSelectIndex);
}

void EditToolbarUI::onKeyDown(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
    if (VK_ESCAPE == wParam)
    {
        ScreenCommon::postNotifyMessage(WM_SNAPSHOT_CANCEL_CPATURE, 0, 0);
    }
}

void EditToolbarUI::onPaint(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{	
	static CxImage toolBarImg;
	static CxImage toolBarHoverImg;
	static CxImage toolBarSelImg;

	BOOL bLoadResult = FALSE;
	if (!bLoadResult)
	{
		toolBarImg.Load(m_sToolbarInfo.m_strPicPath.c_str(), CXIMAGE_SUPPORT_PNG);
		toolBarHoverImg.Load(m_sToolbarInfo.m_strHoverPicPath.c_str(), CXIMAGE_SUPPORT_PNG);
		toolBarSelImg.Load(m_sToolbarInfo.m_strSelPicPath.c_str(), CXIMAGE_SUPPORT_PNG);

		bLoadResult = TRUE;
	}
	
	//draw background
	HDC hPaintDC = GetDC(m_hwnd);
	RECT rcWnd = {0};
	GetClientRect(m_hwnd, &rcWnd);
	toolBarImg.Draw(hPaintDC, rcWnd);

	if (m_iMouseMoveIdx >= 0)
	{
		//draw mouse move
		RECT &rcClip = m_sToolbarInfo.m_sItemRect[m_iMouseMoveIdx];
		toolBarHoverImg.Draw(hPaintDC, rcWnd, &rcClip, true);
	}

	if (m_iSelectIdx >= 0)
	{
		//draw selected
		RECT &rcClip = m_sToolbarInfo.m_sItemRect[m_iSelectIdx];
		toolBarSelImg.Draw(hPaintDC, rcWnd, &rcClip, true);
	}

	ReleaseDC(m_hwnd, hPaintDC);
}

void EditToolbarUI::paintWindow(__in const RECT *rcWnd)
{
	const int DRAW_PEN_WIDTH = 2;

	RECT rc = { 0 };
	memcpy(&rc, rcWnd, sizeof(rc));

	HDC hRectDC = GetDC(m_hwnd);
	HPEN hDrawPen = CreatePen(PS_SOLID, DRAW_PEN_WIDTH, RGB(255, 0, 0));
	HGDIOBJ hPrevPen = SelectObject(hRectDC, hDrawPen);

	int x = rc.left;
	int y = rc.top;
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;

	POINT ptArray[] = { { x, y },{ x + cx, y },{ x + cx, y + cy },{ x, y + cy },{ x, y }};
	Polyline(hRectDC, ptArray, 5);

	SelectObject(hRectDC, (HGDIOBJ)hPrevPen);
	DeleteObject(hDrawPen);
	ReleaseDC(m_hwnd, hRectDC);
}

LPDLGTEMPLATE EditToolbarUI::makeDlgTemplate(LPVOID __in lpBuf)
{
	if (!lpBuf)
	{
		return NULL;
	}
	LPDLGTEMPLATE dlgTemplate = (LPDLGTEMPLATE)ScreenCommon::makeWORDAlign((LPWORD)lpBuf);

	dlgTemplate->style = WS_POPUP | WS_CHILD;
	dlgTemplate->dwExtendedStyle = 0;
	dlgTemplate->cdit = 0;

	dlgTemplate->x = m_sToolbarInfo.m_iX;
	dlgTemplate->y = m_sToolbarInfo.m_iY;
	dlgTemplate->cx = m_sToolbarInfo.m_iWidth;
	dlgTemplate->cy = m_sToolbarInfo.m_iHeight;

	return (LPDLGTEMPLATE)dlgTemplate;
}

HWND EditToolbarUI::createWindow(__in EditToolbarInfo &toolBarInfo, __in HWND hParent/* = NULL*/)
{
	copyToolbarInfo(toolBarInfo);

	BYTE *lpTemplateBuf = new BYTE[1024];
	memset(lpTemplateBuf, 0, 1024);
	HWND hWnd = CreateDialogXIndirect(makeDlgTemplate(lpTemplateBuf), hParent, &EditToolbarUI::DialogProc);

	delete[]lpTemplateBuf;
	return hWnd;
}

void EditToolbarUI::copyToolbarInfo(__in EditToolbarInfo &toolBarInfo)
{
	m_sToolbarInfo.m_iX = toolBarInfo.m_iX;
	m_sToolbarInfo.m_iY = toolBarInfo.m_iY;
	m_sToolbarInfo.m_iWidth = toolBarInfo.m_iWidth;
	m_sToolbarInfo.m_iHeight = toolBarInfo.m_iHeight;
	m_sToolbarInfo.m_strPicPath = toolBarInfo.m_strPicPath;
	m_sToolbarInfo.m_strHoverPicPath = toolBarInfo.m_strHoverPicPath;
	m_sToolbarInfo.m_strSelPicPath = toolBarInfo.m_strSelPicPath;

	m_sToolbarInfo.m_sItemRect.assign(toolBarInfo.m_sItemRect.begin(), toolBarInfo.m_sItemRect.end());
}

int EditToolbarUI::findPtInToolbarRect(__in int xPos, __in int yPos)
{
	POINT pt = {xPos, yPos};
	int iToolbarItemCnt = m_sToolbarInfo.m_sItemRect.size();
	for (int i = 0; i < iToolbarItemCnt; i++)
	{
		LPRECT lpRect = &m_sToolbarInfo.m_sItemRect[i];
		if (!PtInRect(lpRect, pt))
		{
			continue;
		}
		return i;
	}
	return -1;
}

void EditToolbarUI::onLButtonDown(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
	POINT pt = {0};
	GetCursorPos(&pt);
	ScreenToClient(m_hwnd, &pt);

	int rectIdx = findPtInToolbarRect(pt.x, pt.y);
	if (rectIdx >= 0)
	{
		InvalidateRect(m_hwnd, NULL, FALSE);
		m_iSelectIdx = rectIdx;

        ScreenCommon::postNotifyMessage(WM_SNAPSHOT_MODE_CHANGE, 0, 0);
	}
	
	if (_TBI_FINISH == rectIdx )
	{
        ScreenCommon::postNotifyMessage(WM_SNAPSHOT_FINISH_CAPTURE, 0, 0);
	}
    else if (_TBI_CANCEL == rectIdx)
    {
        ScreenCommon::postNotifyMessage(WM_SNAPSHOT_CANCEL_CPATURE, 0, 0);
    }
}

void EditToolbarUI::onMouseMove(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	static int iPrevMouseMoveIdx = -1;
	int rectIdx = findPtInToolbarRect(x, y);
	if ((rectIdx >= 0) && (rectIdx != iPrevMouseMoveIdx))
	{
		m_iMouseMoveIdx = rectIdx;
		iPrevMouseMoveIdx = rectIdx;

		InvalidateRect(m_hwnd, NULL, FALSE);
	}
	else
	{
		m_iMouseMoveIdx = -1;
		iPrevMouseMoveIdx = -1;
	}
}

void EditToolbarUI::onAdjustToolbarPosition(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
	LPRECT lpRect = (LPRECT)wParam;
	BOOL bShow = (BOOL)lParam;

	const int toolBarFloatingGap = 5;
	int toolBarHeight = m_sToolbarInfo.m_iHeight;
	int toolBarWidth = m_sToolbarInfo.m_iWidth;

	static int cxScreen = 0;
	static int cyScreen = 0;
	if (!cyScreen || !cyScreen)
	{
		RECT rcScreen = { 0 };
		GetWindowRect(GetDesktopWindow(), &rcScreen);
		cxScreen = rcScreen.right - rcScreen.left;
		cyScreen = rcScreen.bottom - rcScreen.top;
	}


	RECT rcToolbarRect = { 0 };
	if (lpRect->bottom + toolBarHeight + toolBarFloatingGap <= cyScreen)
	{
		//选择框的下方可以浮动
		rcToolbarRect.left = lpRect->left;
		rcToolbarRect.top = lpRect->bottom + toolBarFloatingGap;
		rcToolbarRect.right = rcToolbarRect.left + toolBarWidth;
		rcToolbarRect.bottom = rcToolbarRect.top + toolBarHeight;
	}
	else if (lpRect->top - toolBarHeight - toolBarFloatingGap >= 0)
	{
		//下方不够地方了，上方有空可以浮动
		rcToolbarRect.left = lpRect->left;
		rcToolbarRect.right = rcToolbarRect.left + toolBarWidth;
		rcToolbarRect.bottom = lpRect->top - toolBarFloatingGap;
		rcToolbarRect.top = rcToolbarRect.bottom - toolBarHeight;
	}
	else
	{
		//实在没地方了，只能放到框里面了,贴到右下角去，同时支持用户自己拖动浮动工具条
		rcToolbarRect.right = lpRect->right;
		rcToolbarRect.bottom = lpRect->bottom;
		rcToolbarRect.left = rcToolbarRect.right - toolBarWidth;
		rcToolbarRect.top = rcToolbarRect.bottom - toolBarHeight;
	}

	MoveWindow(m_hwnd, 
		rcToolbarRect.left, rcToolbarRect.top, 
		rcToolbarRect.right - rcToolbarRect.left, 
		rcToolbarRect.bottom - rcToolbarRect.top, TRUE);

	ShowWindow(m_hwnd, bShow);
}

void EditToolbarUI::onTestSelectIndex(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
	int *pSelectIdx = (int *)wParam;

	*pSelectIdx = m_iSelectIdx;
}

void EditToolbarUI::closeWindow()
{
	m_iSelectIdx = -1;
	m_iMouseMoveIdx = -1;
	m_sToolbarInfo.m_sItemRect.clear();

	m_sToolbarInfo.m_iHeight = 0;
	m_sToolbarInfo.m_iWidth = 0;
	m_sToolbarInfo.m_iX = 0;
	m_sToolbarInfo.m_iY = 0;

    ShowWindow(m_hwnd, SW_HIDE);
	PostMessage(m_hwnd, WM_CLOSE, 0, 0);
}

/******************************************************************************/