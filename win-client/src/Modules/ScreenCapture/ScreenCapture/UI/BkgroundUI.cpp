/******************************************************************************* 
 *  @file      BkgroundUI.cpp 2015\5\5 16:07:13 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "BkgroundUI.h"
#include "../ScreenSnapshot.h"


class CCustomPaintDC
{
public:
	CCustomPaintDC(){}
	~CCustomPaintDC()
	{
		SelectObject(m_hDC, m_hPrevBrush);
		HPEN hDrawPen = (HPEN)SelectObject(m_hDC, (HGDIOBJ)m_hPrevPen);

		DeleteObject(hDrawPen);
		ReleaseDC(m_hWnd, m_hDC);
	}

	CCustomPaintDC(__in HWND hWnd, __in COLORREF iPenColor, __in int iPenWidth)
	{
		m_hWnd = hWnd;
		HPEN hDrawPen = CreatePen(PS_SOLID, iPenWidth, iPenColor);

		m_hDC = GetDC(hWnd);
		m_hPrevBrush = (HBRUSH)SelectObject(m_hDC, GetStockObject(NULL_BRUSH));
		m_hPrevPen = (HPEN)SelectObject(m_hDC, (HGDIOBJ)hDrawPen);
		
	}

	HDC getDC(){ return m_hDC; }

private:
	HWND m_hWnd;
	HDC m_hDC;
	HPEN m_hPrevPen;
	HBRUSH m_hPrevBrush;
};
IMP_DIALOG_PROC(BkgroundUI)

BkgroundUI::BkgroundUI()
{
	memset(&m_sDrawRect, 0, sizeof(RECT));
    memset(&m_sSelectRect, 0, sizeof(RECT));
}

LPDLGTEMPLATE BkgroundUI::makeDlgTemplate(__in LPVOID lpBuf)
{
	if (!lpBuf)
	{
		return NULL;
	}

	LPDLGTEMPLATE dlgTemplate = (LPDLGTEMPLATE)ScreenCommon::makeWORDAlign((LPWORD)lpBuf);

	dlgTemplate->style = WS_POPUP | WS_CHILD;
	dlgTemplate->dwExtendedStyle = 0;
	dlgTemplate->cdit = 0;

	//х╚фа╢╟©з
	RECT rc = {0};
	GetWindowRect(GetDesktopWindow(), &rc);
	dlgTemplate->x = 0;
	dlgTemplate->y = 0;
	dlgTemplate->cx = (short)(rc.right - rc.left);
	dlgTemplate->cy = (short)(rc.bottom - rc.top);

	return (LPDLGTEMPLATE)dlgTemplate;
}

HWND BkgroundUI::createWindow()
{
	BYTE *lpTemplateBuf = new BYTE[1024];
	memset(lpTemplateBuf, 0, 1024);
	HWND hWnd = CreateDialogXIndirect(makeDlgTemplate(lpTemplateBuf), GetDesktopWindow(), &BkgroundUI::DialogProc);

	DWORD dwErr = GetLastError();
	delete[]lpTemplateBuf;

	return hWnd;
}

void BkgroundUI::onInitDialog(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
}

void BkgroundUI::InitMsgMap()
{
	CDlgBase::InitMsgMap();
	RegisterMsg(WM_INITDIALOG, (MSG_HANDLER)&BkgroundUI::onInitDialog);
    RegisterMsg(WM_KEYDOWN, (MSG_HANDLER)&BkgroundUI::onKeyDown);
	RegisterMsg(WM_PAINT, (MSG_HANDLER)&BkgroundUI::onPaint);
    RegisterMsg(WM_SNAPSHOT_MOUSE_WINDOW_CHANGE, (MSG_HANDLER)&BkgroundUI::onSnapshotWindowChange);
    RegisterMsg(WM_SNAPSHOT_TEST_SELECT_RECT, (MSG_HANDLER)&BkgroundUI::onSnapshotTestSelectRect);
	RegisterMsg(WM_SNAPSHOT_DRAW_RECT, (MSG_HANDLER)&BkgroundUI::onDrawRect);
	RegisterMsg(WM_SNAPSHOT_DRAW_POLY_REGION, (MSG_HANDLER)&BkgroundUI::onDrawPolyRegion);
	RegisterMsg(WM_SNAPSHOT_ADD_RECT_ITEM, (MSG_HANDLER)&BkgroundUI::onAddRectItem);
	RegisterMsg(WM_SNAPSHOT_ADD_POLY_REGION_ITEM, (MSG_HANDLER)&BkgroundUI::onAddPolyRegionItem);
}

void BkgroundUI::onKeyDown(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
    if (VK_ESCAPE == wParam)
    {
        ScreenCommon::postNotifyMessage(WM_SNAPSHOT_CANCEL_CPATURE, 0, 0);
    }
}

void BkgroundUI::onSnapshotTestSelectRect(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
    LPRECT lpRect = (LPRECT)wParam;
    if (!lpRect)
    {
        return;
    }
    memcpy(lpRect, &m_sSelectRect, sizeof(RECT));
}
void BkgroundUI::onSnapshotWindowChange(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
    LPRECT lpRect = (LPRECT)lParam;
    if (ScreenCommon::isRectEqual(lpRect, &m_sSelectRect))
    {
        return;
    }
    memcpy(&m_sSelectRect, lpRect, sizeof(RECT));
    ScreenSnapshot::Instance()->paintWndRect(hWnd, &m_sSelectRect);
}

void BkgroundUI::onPaint(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
    ScreenSnapshot::Instance()->paintWndRect(hWnd, &m_sSelectRect);

	if (m_sDrawRect.size())
	{
		for (unsigned int i = 0; i < m_sDrawRect.size(); i++)
		{
			drawRect(m_sDrawRect[i]);
		}
	}

	if (m_sDrawPolyRegion.size())
	{
		for (unsigned int i = 0; i < m_sDrawPolyRegion.size(); i++)
		{
			drawPolyRegion(m_sDrawPolyRegion[i]);
		}
	}

	if (!ScreenCommon::isRectEmpty(m_sCurrentDraw))
	{
		if (m_bDrawRect)
		{
			drawRect(m_sCurrentDraw);
		}
		else
		{
			drawPolyRegion(m_sCurrentDraw);
		}
	}
}

void BkgroundUI::onDrawPolyRegion(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
	m_bDrawRect = FALSE;
	LPRECT lpRect = (LPRECT)wParam;
	memcpy(&m_sCurrentDraw, lpRect, sizeof(RECT));

    ScreenSnapshot::Instance()->paintWndRect(hWnd, &m_sSelectRect);

    drawPolyRegion(m_sCurrentDraw);
    ValidateRect(hWnd, NULL);
}

void BkgroundUI::drawPolyRegion(__in RECT &rc)
{
	CCustomPaintDC paintDC(m_hwnd, RGB(255, 0, 0), 2);
	HDC hPaintDC = paintDC.getDC();

	Ellipse(hPaintDC, rc.left, rc.top, rc.right, rc.bottom);
}

void BkgroundUI::onDrawRect(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
	m_bDrawRect = TRUE;

	LPRECT lpRect = (LPRECT)wParam;
	memcpy(&m_sCurrentDraw, lpRect, sizeof(RECT));

     ScreenSnapshot::Instance()->paintWndRect(hWnd, &m_sSelectRect);

    drawRect(m_sCurrentDraw);
    ValidateRect(hWnd, NULL);
}

void BkgroundUI::drawRect(__in RECT &rc)
{
	CCustomPaintDC paintDC(m_hwnd, RGB(255, 0, 0), 2);
	HDC hPaintDC = paintDC.getDC();

	Rectangle(hPaintDC, rc.left + 1, rc.top + 1, rc.right, rc.bottom);
}

void BkgroundUI::closeWindow()
{
	m_sDrawRect.clear();
	m_sDrawPolyRegion.clear();
	memset(&m_sCurrentDraw, 0, sizeof(RECT));
	m_bDrawRect = FALSE;

    ShowWindow(m_hwnd, SW_HIDE);
	PostMessage(m_hwnd, WM_CLOSE, 0, 0);
}

void BkgroundUI::onAddRectItem(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
	LPRECT lpRect = (LPRECT)wParam;
	RECT rc = { 0 };
	memcpy(&rc, lpRect, sizeof(RECT));
	m_sDrawRect.push_back(rc);

    ScreenSnapshot::Instance()->updateRect(lpRect);
}

void BkgroundUI::onAddPolyRegionItem(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam)
{
	LPRECT lpRect = (LPRECT)wParam;

	RECT rc = { 0 };
	memcpy(&rc, lpRect, sizeof(RECT));
	m_sDrawPolyRegion.push_back(rc);

    ScreenSnapshot::Instance()->updateRect(lpRect);
}


/******************************************************************************/