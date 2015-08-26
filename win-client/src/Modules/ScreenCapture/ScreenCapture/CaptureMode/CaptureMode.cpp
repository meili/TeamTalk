/******************************************************************************* 
 *  @file      CaptureMode.cpp 2015\5\5 16:14:47 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "CaptureMode.h"
#include "AutoSelectMode.h"
#include "ManalSelectMode.h"
#include "ManalEditMode.h"
/******************************************************************************/


CaptureModeMgr::CaptureModeMgr() :
m_emCurrentMode(CM_MIN)
{
}

CaptureMode CaptureModeMgr::changeMode(__in CaptureMode emMode)
{
	CaptureMode emCurrentMode = m_emCurrentMode;
	m_emCurrentMode = emMode;

	return emCurrentMode;
}

void CaptureModeMgr::handleMouseMsg(__in UINT uMsg, __in int xPos, __in int yPos)
{
	IModeMsgHandler *msgHandler = getModeHandler();
	if (!msgHandler) return;

	if (WM_MOUSEMOVE == uMsg)
	{
		msgHandler->onMouseMove(xPos, yPos);
	}
	else if (WM_LBUTTONDOWN == uMsg)
	{
		msgHandler->onLButtonDown(xPos, yPos);
	}
	else if (WM_LBUTTONUP == uMsg)
	{
		msgHandler->onLButtonUp(xPos, yPos);
	}
	else if (WM_LBUTTONDBLCLK == uMsg)
	{
		msgHandler->onLButtonDBClick(xPos, yPos);
	}
}

IModeMsgHandler *CaptureModeMgr::getModeHandler()
{
	if (CM_AUTO_SELECT == m_emCurrentMode)
	{
		return AutoSelectMode::Instance();
	}
	else if (CM_MANAL_SELECT == m_emCurrentMode)
	{
		return ManalSelectMode::Instance();
	}
	else if (CM_MANAL_EDIT == m_emCurrentMode)
	{
		return ManalEditMode::Instance();
	}

	return NULL;
}

BOOL CaptureModeMgr::snapshotWindows()
{
	return AutoSelectMode::Instance()->snapshotWindows();
}

void CaptureModeMgr::clear()
{
    m_emCurrentMode = CM_MIN;
	AutoSelectMode::Instance()->clear();
	ManalSelectMode::Instance()->clear();
	ManalEditMode::Instance()->clear();
}

/******************************************************************************/