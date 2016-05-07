/*******************************************************************************
 *  @file      ManalSelectMode.h 2015\5\5 16:09:38 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef MANALSELECTMODE_08E677DD_16A3_4241_BED9_479B4DD4DE34_H__
#define MANALSELECTMODE_08E677DD_16A3_4241_BED9_479B4DD4DE34_H__

#include "ModeComm.h"
/******************************************************************************/

/**
 * The class <code>ManalSelectMode</code> 
 *
 */

class ManalSelectMode :public Singleton<ManalSelectMode>, public IModeMsgHandler
{
public:
	ManalSelectMode();
	void clear();

public:
	virtual void onLButtonDown(__in int x, __in int y);
	virtual void onLButtonUp(__in int x, __in int y);
	virtual void onLButtonDBClick(__in int x, __in int y);
	virtual void onMouseMove(__in int x, __in int y);

public:
	void adjustToolbarPosition(__in BOOL bShow);

private:
	int m_iPosX;
	int m_iPosY;
	int m_iClickXDis;
	int m_iClickYDis;
	RECT m_sSelectRect;
	BOOL m_bSelectFinish;
};

/******************************************************************************/
#endif// MANALSELECTMODE_08e677dd-16a3-4241-bed9-479b4dd4de34_H__

