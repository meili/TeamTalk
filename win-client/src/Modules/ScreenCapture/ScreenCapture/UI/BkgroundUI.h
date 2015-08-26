
/*******************************************************************************
 *  @file      BkgroundUI.h 2015\5\5 16:05:50 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef BKGROUNDUI_30A15831_BD3E_4E94_817A_E1D46B604B91_H__
#define BKGROUNDUI_30A15831_BD3E_4E94_817A_E1D46B604B91_H__
#include "../Common.h"
#include "Dlgbase.h"

/******************************************************************************/

/**
 * The class <code>BkgroundUI</code> 
 *
 */
;

class BkgroundUI :public CDlgBase, public Singleton<BkgroundUI>
{
public:
	BkgroundUI();
	HWND createWindow();
	void closeWindow();

private:
	LPDLGTEMPLATE makeDlgTemplate(__in LPVOID lpBuf);

private:
	void InitMsgMap();
	void onInitDialog(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
    void onKeyDown(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
    void onSnapshotTestSelectRect(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
    void onSnapshotWindowChange(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
	void onPaint(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
	void onDrawRect(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
	void onDrawPolyRegion(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
	void onAddRectItem(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
	void onAddPolyRegionItem(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);

	DECLARE_DIALOG_PROC

private:
	void drawRect(__in RECT &rc);
	void drawPolyRegion(__in RECT &rc);

private:
    RECT m_sSelectRect;

private:
	BOOL m_bDrawRect;
	RECT m_sCurrentDraw;
	std::vector<RECT> m_sDrawRect;
	std::vector<RECT> m_sDrawPolyRegion;
};

/******************************************************************************/
#endif// BKGROUNDUI_30a15831-bd3e-4e94-817a-e1d46b604b91_H__
