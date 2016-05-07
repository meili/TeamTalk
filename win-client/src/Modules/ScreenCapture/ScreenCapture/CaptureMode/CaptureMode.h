/*******************************************************************************
 *  @file      CaptureMode.h 2015\5\5 16:14:14 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef CAPTUREMODE_1BD9073C_B1F7_4892_BD99_555BD9D973B7_H__
#define CAPTUREMODE_1BD9073C_B1F7_4892_BD99_555BD9D973B7_H__

#include "ModeComm.h"
/******************************************************************************/

/**
 * The class <code>CaptureMode</code> 
 *
 */

enum CaptureMode
{
	CM_MIN = 0,
	CM_AUTO_SELECT,
	CM_MANAL_SELECT,
	CM_MANAL_EDIT,

	CM_MAX
};

class CaptureModeMgr :public Singleton<CaptureModeMgr>
{
public:
	CaptureModeMgr();
	CaptureMode getMode(){ return m_emCurrentMode; }
	CaptureMode changeMode(__in CaptureMode emMode);
	void clear();

public:
	BOOL snapshotWindows();

public:
	void handleMouseMsg(__in UINT uMsg, __in int xPos, __in int yPos);

private:
	IModeMsgHandler *getModeHandler();

private:
	CaptureMode m_emCurrentMode;
};


/******************************************************************************/
#endif// CAPTUREMODE_1bd9073c-b1f7-4892-bd99-555bd9d973b7_H__
