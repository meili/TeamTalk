/******************************************************************************* 
 *  @file      ManalSelectMode.cpp 2015\5\5 16:10:38 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "ManalSelectMode.h"
#include "../UI/UIMgr.h"

/******************************************************************************/
ManalSelectMode::ManalSelectMode()
{
	
}

void ManalSelectMode::onLButtonDown(__in int x, __in int y)
{
	m_iPosX = x;
	m_iPosY = y;

	if (m_bSelectFinish)
	{
		m_iClickXDis = x - m_sSelectRect.left;
		m_iClickYDis = y - m_sSelectRect.top;
	}
}

void ManalSelectMode::onLButtonUp(__in int x, __in int y)
{
	if (!ScreenCommon::isRectEmpty(m_sSelectRect))
	{
		UIScreenCaptureMgr::Instance()->sendEditToolbarMessage(WM_SNAPSHOT_ADJUST_TOOL_BAR_POSITION, (WPARAM)&m_sSelectRect, (LPARAM)TRUE);		
		m_bSelectFinish = TRUE;
	}
	else
	{
		UIScreenCaptureMgr::Instance()->sendBkgMessage(WM_SNAPSHOT_TEST_SELECT_RECT, (WPARAM)&m_sSelectRect, 0);
		if (!ScreenCommon::isRectEmpty(m_sSelectRect))
		{
			UIScreenCaptureMgr::Instance()->sendEditToolbarMessage(WM_SNAPSHOT_ADJUST_TOOL_BAR_POSITION, (WPARAM)&m_sSelectRect, (LPARAM)TRUE);
			m_bSelectFinish = TRUE;
		}
		else
		{
			UIScreenCaptureMgr::Instance()->sendEditToolbarMessage(WM_SNAPSHOT_ADJUST_TOOL_BAR_POSITION, (WPARAM)&m_sSelectRect, (LPARAM)FALSE);
			m_bSelectFinish = FALSE;
		}
	}
}

void ManalSelectMode::onLButtonDBClick(__in int x, __in int y)
{

}

void ManalSelectMode::onMouseMove(__in int x, __in int y)
{
	static int xPrev = 0;
	static int yPrev = 0;
	if (x == xPrev && y == yPrev)
	{
		return;
	}
	xPrev = x;
	yPrev = y;

	if (m_bSelectFinish)
	{
		POINT pt = { x, y };
		if (PtInRect(&m_sSelectRect, pt))
		{
			UIScreenCaptureMgr::Instance()->changeBkCursor(TRUE);
		}
		else
		{
			UIScreenCaptureMgr::Instance()->changeBkCursor(FALSE);
		}
	}

	if (!ScreenCommon::isLButtonDown())
	{
		return;
	}

	if (m_bSelectFinish)
	{
		int cx = m_sSelectRect.right - m_sSelectRect.left;
		int cy = m_sSelectRect.bottom - m_sSelectRect.top;

		RECT rcDesktop = {0};
		GetWindowRect(GetDesktopWindow(), &rcDesktop);

		m_sSelectRect.left = (x - m_iClickXDis);
		if (m_sSelectRect.left < 0)
		{
			m_sSelectRect.left = 0;
		}
		if (m_sSelectRect.left + cx > rcDesktop.right)
		{
			m_sSelectRect.left = rcDesktop.right - cx;
		}

		m_sSelectRect.right = (m_sSelectRect.left + cx);
		m_sSelectRect.top = (y - m_iClickYDis);
		if (m_sSelectRect.top < 0)
		{
			m_sSelectRect.top = 0;
		}
		if (m_sSelectRect.top + cy > rcDesktop.bottom)
		{
			m_sSelectRect.top = rcDesktop.bottom - cy;
		}
		m_sSelectRect.bottom = (m_sSelectRect.top + cy);

		UIScreenCaptureMgr::Instance()->sendBkgMessage(WM_SNAPSHOT_MOUSE_WINDOW_CHANGE, 0, (LPARAM)&m_sSelectRect);
		UIScreenCaptureMgr::Instance()->sendEditToolbarMessage(WM_SNAPSHOT_ADJUST_TOOL_BAR_POSITION, (WPARAM)&m_sSelectRect, (LPARAM)TRUE);
	}
	else
	{
		RECT rc = { m_iPosX, m_iPosY, x, y };
		UIScreenCaptureMgr::Instance()->sendBkgMessage(WM_SNAPSHOT_MOUSE_WINDOW_CHANGE, 0, (LPARAM)&rc);
		m_sSelectRect = rc;
	}
}

void ManalSelectMode::clear()
{
	m_iPosX = -1;
	m_iPosY = -1;
	
	m_iClickXDis = -1;
	m_iClickYDis = -1;

	memset(&m_sSelectRect, 0, sizeof(RECT));
	m_bSelectFinish = FALSE;
}


/******************************************************************************/

