
/*******************************************************************************
 *  @file      EditToolbarUI.h 2015\5\5 16:04:05 $
 *  @author    доки<nansong@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef EDITTOOLBARUI_626E8E45_6C7F_4273_8469_1B3BEB73CC36_H__
#define EDITTOOLBARUI_626E8E45_6C7F_4273_8469_1B3BEB73CC36_H__

#include "../Common.h"
#include "Dlgbase.h"
/******************************************************************************/

/**
 * The class <code>EditToolbarUI</code> 
 *
 */

struct EditToolbarInfo
{
	int m_iX;
	int m_iY;
	int m_iWidth;
	int m_iHeight;
	std::wstring m_strPicPath;
	std::wstring m_strHoverPicPath;
	std::wstring m_strSelPicPath;
	std::vector<RECT> m_sItemRect;
};

enum ToolBarButtonIdx
{
	_TBI_RECT,
	_TBI_ELLIPSE,
	_TBI_CANCEL,
	_TBI_FINISH,

	_TBI_MAX
};

class EditToolbarUI : public CDlgBase, public Singleton<EditToolbarUI>
{
public:
	EditToolbarUI();
	HWND createWindow(__in EditToolbarInfo &toolBarInfo, __in HWND hParent = NULL);
	void closeWindow();

private:
	LPDLGTEMPLATE makeDlgTemplate(__in LPVOID lpBuf);

private:
	void paintWindow(__in const RECT *rc);
	void copyToolbarInfo(__in EditToolbarInfo &toolBarInfo);
	
private:
	int findPtInToolbarRect(__in int xPos, __in int yPos);

public:
	void InitMsgMap();
	void onPaint(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
    void onKeyDown(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
	void onLButtonDown(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
	void onMouseMove(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
	void onPaintToolBarItem(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
	void onTestSelectIndex(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);
	void onAdjustToolbarPosition(__in HWND hWnd, __in WPARAM wParam, __in LPARAM lParam);

	DECLARE_DIALOG_PROC

private:
	EditToolbarInfo m_sToolbarInfo;
	int m_iSelectIdx;
	int m_iMouseMoveIdx;
};

/******************************************************************************/
#endif// EDITTOOLBARUI_626e8e45-6c7f-4273-8469-1b3beb73cc36_H__