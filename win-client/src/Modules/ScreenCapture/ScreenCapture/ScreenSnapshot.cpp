
/******************************************************************************* 
 *  @file      ScreenSnapshot.cpp 2015\5\5 15:54:05 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "ScreenSnapshot.h"
#include "../../3rdParty/src/cxImage/cxImage/ximage.h"

/******************************************************************************/

ScreenSnapshot::ScreenSnapshot()
:m_hMemDC(NULL),
m_hBkgMemDC(NULL),
m_hDrawMemDC(NULL)
{}

BOOL ScreenSnapshot::updateRect(__in RECT *rc)
{
    if (!rc)
    {
        return FALSE;
    }

    if (ScreenCommon::isRectEmpty(*rc))
    {
        return FALSE;
    }

    HWND hDesktopWnd = GetDesktopWindow();
    HDC hScreenDC = GetDC(hDesktopWnd);

    RECT rcWnd = { 0 };
    GetWindowRect(hDesktopWnd, &rcWnd);
    int cx = rcWnd.right - rcWnd.left;
    int cy = rcWnd.bottom - rcWnd.top;

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, cx, cy);
    HDC hMemDC = CreateCompatibleDC(hScreenDC);

    HGDIOBJ hOldBitmap = SelectObject(hMemDC, (HGDIOBJ)hBitmap);
    BitBlt(hMemDC, 0, 0, cx, cy, hScreenDC, 0, 0, SRCCOPY);

    //update to m_hMemDC
    int updateX = rc->left;
    int updateY = rc->top;
    int updateCx = rc->right - rc->left;
    int updateCy = rc->bottom - rc->top;

    BitBlt(m_hMemDC, updateX, updateY, updateCx, updateCy, hMemDC, updateX, updateY, SRCCOPY);

    DeleteDC(hMemDC);
    DeleteObject(hBitmap);
    ReleaseDC(hDesktopWnd, hScreenDC);

    return TRUE;
}

BOOL ScreenSnapshot::snapshotScreen()
{
	if (m_hMemDC)
	{
        DeleteObject(m_hMemBitmap);
		DeleteDC(m_hMemDC);
        m_hMemBitmap = NULL;
		m_hMemDC = NULL;
	}
    if (m_hBkgMemDC)
    {
        DeleteObject(m_hBkBitmap);
        DeleteDC(m_hBkgMemDC);
        m_hBkBitmap = NULL;
        m_hBkgMemDC = NULL;
    }
    if (m_hDrawMemDC)
    {
        DeleteObject(m_hDrawBitmap);
        DeleteDC(m_hDrawMemDC);
        m_hDrawBitmap = NULL;
        m_hDrawMemDC = NULL;
    }

	//
	HWND hDesktopWnd = GetDesktopWindow();
	HDC hScreenDC = GetDC(hDesktopWnd);

	RECT rc = { 0 };
	GetWindowRect(hDesktopWnd, &rc);
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;

	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, cx, cy);
	m_hMemDC = CreateCompatibleDC(hScreenDC);
	HGDIOBJ hOldBitmap = SelectObject(m_hMemDC, (HGDIOBJ)hBitmap);
	BitBlt(m_hMemDC, 0, 0, cx, cy, hScreenDC, 0, 0, SRCCOPY);

    m_hBkgMemDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBkgBitmap = CreateCompatibleBitmap(hScreenDC, cx, cy);
    SelectObject(m_hBkgMemDC, (HGDIOBJ)hBkgBitmap);
    BitBlt(m_hBkgMemDC, 0, 0, cx, cy, hScreenDC, 0, 0, SRCCOPY);

    HDC hMaskDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hMaskBitmap = CreateCompatibleBitmap(hScreenDC, cx, cy);
    SelectObject(hMaskDC, (HGDIOBJ)hMaskBitmap);

    BLENDFUNCTION ftn = { AC_SRC_OVER, 0, 100, 0};
    AlphaBlend(m_hBkgMemDC, 0, 0, cx, cy, hMaskDC, 0, 0, cx, cy, ftn);
    DeleteObject(hMaskBitmap);
    DeleteDC(hMaskDC);

    m_hDrawMemDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hDrawBitmap = CreateCompatibleBitmap(hScreenDC, cx, cy);
    SelectObject(m_hDrawMemDC, hDrawBitmap);

	ReleaseDC(hDesktopWnd, hScreenDC);
	return TRUE;
}

BOOL ScreenSnapshot::paintWndRect(__in HWND hWnd, __in LPRECT lpRect)
{
    RECT rcWnd = { 0 };
    GetWindowRect(hWnd, &rcWnd);
    int cxWnd = rcWnd.right - rcWnd.left;
    int cyWnd = rcWnd.bottom - rcWnd.top;

	HDC hWndDC = GetDC(hWnd);

    //1. draw background
	BitBlt(m_hDrawMemDC, 0, 0, cxWnd, cxWnd, m_hBkgMemDC, 0, 0, SRCCOPY);

    //2. draw high light rect if any
    if (lpRect)
    {
        int cx = lpRect->right - lpRect->left;
        int cy = lpRect->bottom - lpRect->top;
        POINT pt = { lpRect->left, lpRect->top };
        BitBlt(m_hDrawMemDC, pt.x, pt.y, cx, cy, m_hMemDC, pt.x, pt.y, SRCCOPY);

        //3. now draw green rect
        HBRUSH hPrevBrush = (HBRUSH)SelectObject(m_hDrawMemDC, (HGDIOBJ)GetStockObject(NULL_BRUSH));
        HPEN hDrawPen = CreatePen(PS_SOLID, m_iDrawPenWidth, RGB(255, 215, 0));
        HPEN hPrevPen = (HPEN)SelectObject(m_hDrawMemDC, hDrawPen);
        Rectangle(m_hDrawMemDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
        SelectObject(m_hDrawMemDC, hPrevBrush);
        SelectObject(m_hDrawMemDC, hPrevPen);
    }
    BitBlt(hWndDC, 0, 0, cxWnd, cxWnd, m_hDrawMemDC, 0, 0, SRCCOPY);

	ReleaseDC(hWnd, hWndDC);
    ValidateRect(hWnd, NULL);

	return TRUE;
}

BOOL ScreenSnapshot::saveRect(__in RECT &rc, __in std::wstring &savePath)
{
	snapshotScreen();

	CxImage img;
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;

	HDC hSaveDC = CreateCompatibleDC(m_hMemDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(m_hMemDC, cx, cy);
	HBITMAP hSaveBitmap = (HBITMAP)SelectObject(hSaveDC, (HGDIOBJ)hBitmap);
	BitBlt(hSaveDC, 0, 0, cx, cy, m_hMemDC, rc.left, rc.top, SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hSaveDC, (HBITMAP)hSaveBitmap);
	
	BOOL result = FALSE;
	do 
	{
		if (!img.CreateFromHBITMAP(hBitmap))
		{
			break;
		}
		if (!img.Save(savePath.c_str(), CXIMAGE_FORMAT_BMP))
		{
			break;
		}
		result = TRUE;
	} while (FALSE);

	DeleteObject((HGDIOBJ)hBitmap);
	DeleteDC(hSaveDC);

	return result;
}

/******************************************************************************/