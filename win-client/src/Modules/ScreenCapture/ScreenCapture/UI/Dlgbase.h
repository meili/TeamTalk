/*
    对话框基础类
*/
#pragma once

#include <shlwapi.h>
#include <vector>

#define DECLARE_DIALOG_PROC \
	static INT_PTR CALLBACK DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );//主窗口函数

#define IMP_DIALOG_PROC( ClassName ) \
	INT_PTR CALLBACK ClassName##::DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) \
	{ \
	return ClassName##::Instance()->DialogProc_Internal( hwndDlg, uMsg, wParam, lParam ); \
	}

typedef INT_PTR ( CALLBACK *DIALOGPROC )( HWND, UINT, WPARAM, LPARAM );
class CDlgBase
{ 
public:
    CDlgBase();
	~CDlgBase();
	
	//-----------------------------------相关类型定义--------------------------------------------------------
	typedef void ( CDlgBase:: *MSG_HANDLER )( HWND, WPARAM, LPARAM ); //定义消息处理函数类型
	typedef LRESULT ( CDlgBase:: *NOTIFY_HANDLER )( HWND, HWND, WPARAM, LPARAM );// WM_NOTIFY的子处理函数
	typedef void ( CDlgBase:: *COMMAND_HANDLER )( HWND, WPARAM, LPARAM );//WM_COMMAND 的具体ID的处理函数 
	typedef void ( CDlgBase:: *COMMAND_HANDLER_EX )( HWND, HWND,WPARAM, LPARAM );//WM_COMMANDEX 的具体ID的处理函数 

	struct _NOTIFY_MSG
	{
		int m_iCtrlID;
		UINT m_iNotifyCode;
		NOTIFY_HANDLER m_pHandler;

		_NOTIFY_MSG()
		{
			m_iCtrlID = 0;
			m_iNotifyCode = 0;
			m_pHandler = NULL;
		}

		_NOTIFY_MSG( int iCtrlID, UINT iNotifyCode, NOTIFY_HANDLER pHandler )
		{
			m_iCtrlID = iCtrlID;
			m_iNotifyCode = iNotifyCode;
			m_pHandler = pHandler;
		}
	};
	struct  _COMMAND_MSG_EX
	{
		int m_iCmdID;
		UINT m_iNotifyCode;
		COMMAND_HANDLER_EX m_pHandler;

		_COMMAND_MSG_EX()
		{
			m_iCmdID = 0;
			m_iNotifyCode = 0;
			m_pHandler = NULL;
		}

		_COMMAND_MSG_EX( int iCmdID, int iNotifyCode, COMMAND_HANDLER_EX pHandler )
		{
			m_iCmdID = iCmdID;
			m_iNotifyCode = iNotifyCode;
			m_pHandler = pHandler;
		}

	};

	struct  _COMMAND_MSG
	{
		int m_iCmdID;
		COMMAND_HANDLER m_pHandler;

		_COMMAND_MSG()
		{
			m_iCmdID = 0;
			m_pHandler = NULL;
		}

		_COMMAND_MSG( int iCmdID, COMMAND_HANDLER pHandler )
		{
			m_iCmdID = iCmdID;
			m_pHandler = pHandler;
		}

	};

	struct _WM_MSG
	{
		int m_iMsgID;
		MSG_HANDLER m_pHandler;

		_WM_MSG()
		{
			m_iMsgID = 0;
			m_pHandler = NULL;
		}

		_WM_MSG( int iMsgID, MSG_HANDLER pHandler )
		{
			m_iMsgID = iMsgID;
			m_pHandler = pHandler;
		}
	};
	//-------------------------------------------------------------------------------------------
	virtual void RegisterMsg( int iMsgID, MSG_HANDLER pfnMsgHandler );
	virtual void RegisterCmd( int iCmdID, COMMAND_HANDLER pfnCmdHandler );
	virtual void RegisterCmdEx( int iCtrlID, int iNotifyCode, COMMAND_HANDLER_EX pfnCmdHandler );
	virtual void RegisterNotify( int iCtrlID, int iNotifyCode, NOTIFY_HANDLER pfnNotifyHandler );
	virtual void DoModal( int iIDD_DLG, HWND hParant, DIALOGPROC pDialogProc );//模态对话框
	virtual void DoModal( HINSTANCE hInstance, int iIDD_DLG, HWND hParant, DIALOGPROC pDialogProc );
	virtual void DoModal( int iIDD_DLG, HWND hParant, LPARAM lParam, DIALOGPROC pDialogProc );//带参数的模态对话框
	virtual void DoModal( HINSTANCE hInstance, int iIDD_DLG, HWND hParant, LPARAM lParam, DIALOGPROC pDialogProc );
	virtual HWND CreateDialogX( int iIDD_DLG, HWND hParant, DIALOGPROC pDialogProc );//非模态对话框 
	virtual HWND CreateDialogX( HINSTANCE hInstatnce, int iIDD_DLG, HWND hParant, DIALOGPROC pDialogProc );//非模态对话框 
	virtual HWND CreateDialogXIndirect(LPDLGTEMPLATE lpTemplate, HWND hParent, DIALOGPROC pDialogProc);//使用模板结构直接创建
	virtual HWND GetHWND(){ return m_hwnd; };
	virtual LRESULT SendMsg( UINT Msg, WPARAM wParam, LPARAM lParam );
	
	virtual void InitMsgMap();//初始化消息映射
	virtual INT_PTR DialogProc_Internal( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	virtual void OnClose( HWND hWnd, WPARAM wParam, LPARAM lParam );

	std::vector<_WM_MSG> m_WM_MsgMap;
	std::vector<_COMMAND_MSG> m_Cmd_MsgMap;
	std::vector<_NOTIFY_MSG> m_Notify_MsgMap;
	std::vector<_COMMAND_MSG_EX> m_Cmd_Ex_MsgMap;
	HWND m_hwnd;
	INT_PTR m_DialogRet;
	
};