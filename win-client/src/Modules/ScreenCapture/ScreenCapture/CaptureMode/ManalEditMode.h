/*******************************************************************************
 *  @file      ManalEditMode.h 2015\5\5 16:11:11 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef MANALEDITMODE_380DA203_9530_4BA1_8DAC_3A032EF27567_H__
#define MANALEDITMODE_380DA203_9530_4BA1_8DAC_3A032EF27567_H__

#include "ModeComm.h"
/******************************************************************************/

/**
 * The class <code>ManalEditMode</code> 
 *
 */

class ManalEditMode :public Singleton<ManalEditMode>, public IModeMsgHandler
{
public:
	ManalEditMode();
	void clear();
public:
	virtual void onLButtonDown(__in int x, __in int y);
	virtual void onLButtonUp(__in int x, __in int y);
	virtual void onLButtonDBClick(__in int x, __in int y);
	virtual void onMouseMove(__in int x, __in int y);
	virtual void saveSelectRect(__in std::wstring savePath);

private:
	void onRectangle(__in int x, __in int y);
	void onPolyRegion(__in int x, __in int y);
	void onFreeRegion(__in int x, __in int y);

private:
	int m_iSelectIdx;
	int m_iPosX;
	int m_iPosY;
	RECT m_sSelectRect;
};

/******************************************************************************/
#endif// MANALEDITMODE_380da203-9530-4ba1-8dac-3a032ef27567_H__
