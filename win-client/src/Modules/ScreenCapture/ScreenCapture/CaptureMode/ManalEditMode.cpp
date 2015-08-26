
/******************************************************************************* 
 *  @file      ManalEditMode.cpp 2015\5\5 16:13:13 $
 *  @author    南松<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "ManalEditMode.h"
#include "../UI/UIMgr.h"
#include "../ScreenSnapshot.h"

/******************************************************************************/

ManalEditMode::ManalEditMode()
:m_iSelectIdx(-1)
{
	memset(&m_sSelectRect, 0, sizeof(RECT));
}

void ManalEditMode::saveSelectRect(__in std::wstring savePath)
{
	ScreenSnapshot::Instance()->saveRect(m_sSelectRect, savePath);
}
void ManalEditMode::onLButtonDown(__in int x, __in int y)
{
	UIScreenCaptureMgr::Instance()->sendEditToolbarMessage(WM_SNAPSHOT_TEST_SELECT_IDX, (WPARAM)&m_iSelectIdx, (LPARAM)&y);
	UIScreenCaptureMgr::Instance()->sendBkgMessage(WM_SNAPSHOT_TEST_SELECT_RECT, (WPARAM)&m_sSelectRect, 0);

	POINT pt = { x, y };
	if (!PtInRect(&m_sSelectRect, pt))
	{
		if (x < m_sSelectRect.left)
		{
			x = m_sSelectRect.left;
		}
		if (y < m_sSelectRect.top)
		{
			y = m_sSelectRect.top;
		}

		if (x > m_sSelectRect.right || y > m_sSelectRect.bottom)
		{
			m_iPosX = -1;
			m_iPosY = -1;
			return;
		}
	}

	m_iPosX = x;
	m_iPosY = y;
}

void ManalEditMode::onLButtonUp(__in int x, __in int y)
{
	POINT beginPT = {m_iPosX, m_iPosY};
	if (!PtInRect(&m_sSelectRect, beginPT))
	{
		return;
	}

	POINT pt = { x, y };
	if (!PtInRect(&m_sSelectRect, pt))
	{
		if (x > m_sSelectRect.right)
		{
			x = m_sSelectRect.right;
		}
		if (y > m_sSelectRect.bottom)
		{
			y = m_sSelectRect.bottom;
		}
		if (x < m_sSelectRect.left || y < m_sSelectRect.top)
		{
			return;
		}
	}
	//
	RECT rc = {m_iPosX, m_iPosY, x, y};
	if (0 == m_iSelectIdx)
	{
		UIScreenCaptureMgr::Instance()->sendBkgMessage(WM_SNAPSHOT_ADD_RECT_ITEM, (WPARAM)&rc, 0);
	}
	else
	{
		UIScreenCaptureMgr::Instance()->sendBkgMessage(WM_SNAPSHOT_ADD_POLY_REGION_ITEM, (WPARAM)&rc, 0);
	}
	
}

void ManalEditMode::onLButtonDBClick(__in int x, __in int y)
{
	//ignore
}


void ManalEditMode::onMouseMove(__in int x, __in int y)
{
	if (m_iSelectIdx < 0)
	{
		return;
	}

	if (!ScreenCommon::isLButtonDown())
	{
		return;
	}

	POINT beginPT = {m_iPosX, m_iPosY};
	if (!PtInRect(&m_sSelectRect, beginPT))
	{
		return;
	}

	POINT pt = {x, y};
	if (!PtInRect(&m_sSelectRect, pt))
	{
		//
		if (x > m_sSelectRect.right)
		{
			x = m_sSelectRect.right;
		}
		if (y > m_sSelectRect.bottom)
		{
			y = m_sSelectRect.bottom;
		}
		if (x < m_sSelectRect.left || y < m_sSelectRect.top)
		{
			return;
		}
	}

	if (0 == m_iSelectIdx) //矩形
	{
		onRectangle(x, y);
	}
	else if (1 == m_iSelectIdx)//椭圆
	{
		onPolyRegion(x, y);
	}
	else if (2 == m_iSelectIdx)//自由画
	{
		onFreeRegion(x, y);
	}
}

void ManalEditMode::onRectangle(__in int x, __in int y)
{
	RECT drawRect = {m_iPosX, m_iPosY, x, y};
	UIScreenCaptureMgr::Instance()->sendBkgMessage(WM_SNAPSHOT_DRAW_RECT, (WPARAM)&drawRect, 0);
}

void ManalEditMode::onPolyRegion(__in int x, __in int y)
{
	RECT drawRect = { m_iPosX, m_iPosY, x, y };
	UIScreenCaptureMgr::Instance()->sendBkgMessage(WM_SNAPSHOT_DRAW_POLY_REGION, (WPARAM)&drawRect, 0);
}

void ManalEditMode::onFreeRegion(__in int x, __in int y)
{

}

void ManalEditMode::clear()
{
	m_iSelectIdx = -1;
	m_iPosX = -1;
	m_iPosY = -1;
	memset(&m_sSelectRect, 0, sizeof(RECT));
}


/******************************************************************************/