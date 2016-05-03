
#include "stdafx.h"
#include "Dlgbase.h"

CDlgBase::CDlgBase()
{
	m_Cmd_MsgMap.clear();
	m_WM_MsgMap.clear();
	m_Notify_MsgMap.clear();

	m_hwnd = NULL;
	m_DialogRet = 0;
}

CDlgBase::~CDlgBase()
{

}

LRESULT CDlgBase::SendMsg( UINT Msg, WPARAM wParam, LPARAM lParam )
{
	//发送消息给自己的窗口函数
	return ::SendMessage( m_hwnd, Msg, wParam, lParam );
}

void CDlgBase::DoModal( int iIDD_DLG, HWND hParant, DIALOGPROC pDialogProc )
{
	InitMsgMap();//初始化消息机制
	m_DialogRet = DialogBox( GetModuleHandle( NULL ), MAKEINTRESOURCE( iIDD_DLG ), hParant, pDialogProc );
}

void CDlgBase::DoModal( HINSTANCE hInstance, int iIDD_DLG, HWND hParant, DIALOGPROC pDialogProc )
{
	InitMsgMap();//初始化消息机制
	m_DialogRet = DialogBox( hInstance, MAKEINTRESOURCE( iIDD_DLG ), hParant, pDialogProc );
	//DWORD dwErr = GetLastError();

	//int a = 0;
}

void CDlgBase::DoModal( int iIDD_DLG, HWND hParant, LPARAM lParam, DIALOGPROC pDialogProc )
{
	InitMsgMap();//初始化消息机制
	m_DialogRet = DialogBoxParam( GetModuleHandle( NULL ), MAKEINTRESOURCE( iIDD_DLG ), hParant, pDialogProc, lParam );
}

void CDlgBase::DoModal( HINSTANCE hInstance, int iIDD_DLG, HWND hParant, LPARAM lParam, DIALOGPROC pDialogProc )
{
	InitMsgMap();//初始化消息机制
	m_DialogRet = DialogBoxParam( hInstance, MAKEINTRESOURCE( iIDD_DLG ), hParant, pDialogProc, lParam );
}


HWND CDlgBase::CreateDialogX( int iIDD_DLG, HWND hParent, DIALOGPROC pDialogProc )
{
	InitMsgMap();//初始化消息机制
	m_hwnd = CreateDialog( GetModuleHandle( NULL ), MAKEINTRESOURCE( iIDD_DLG ), hParent, pDialogProc );
	return m_hwnd;
}
HWND CDlgBase::CreateDialogX( HINSTANCE hInstance, int iIDD_DLG, HWND hParent, DIALOGPROC pDialogProc )
{
	InitMsgMap();//初始化消息机制
	m_hwnd = CreateDialog( hInstance, MAKEINTRESOURCE( iIDD_DLG ), hParent, pDialogProc );
	return m_hwnd;
}
HWND CDlgBase::CreateDialogXIndirect(LPDLGTEMPLATE lpTemplate, HWND hParent, DIALOGPROC pDialogProc)
{
	InitMsgMap();
	m_hwnd = CreateDialogIndirect(GetModuleHandle(NULL), lpTemplate, hParent, pDialogProc);

	DWORD dwErr = GetLastError();
	return m_hwnd;
}
void CDlgBase::OnClose( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	EndDialog( hWnd, m_DialogRet );
}
void CDlgBase::InitMsgMap()
{
	RegisterMsg( WM_CLOSE, ( MSG_HANDLER )&CDlgBase::OnClose );
}
void CDlgBase::RegisterMsg( int iMsgID, MSG_HANDLER pfhMsgHandler )
{
	m_WM_MsgMap.push_back( _WM_MSG( iMsgID, pfhMsgHandler ) );
}
void CDlgBase::RegisterCmd( int iCmdID, COMMAND_HANDLER pfnCmdHandler )
{
	m_Cmd_MsgMap.push_back( _COMMAND_MSG( iCmdID, pfnCmdHandler ) );
}
void CDlgBase::RegisterNotify( int iCtrlID, int iNotifyCode, NOTIFY_HANDLER pfnNotifyHandler )
{
	m_Notify_MsgMap.push_back( _NOTIFY_MSG( iCtrlID, iNotifyCode, pfnNotifyHandler) );
}

void CDlgBase::RegisterCmdEx( int iCtrlID, int iNotifyCode, COMMAND_HANDLER_EX pfnCmdHandler )
{
	m_Cmd_Ex_MsgMap.push_back( _COMMAND_MSG_EX( iCtrlID, iNotifyCode, pfnCmdHandler ) );
}
//INT_PTR CALLBACK CDlgBase::DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
//{
//	if( m_pThisBase )
//	{
//		return m_pThisBase->DialogProc_Internal( hwndDlg, uMsg, wParam, lParam );
//	}
//	return 0;
//}
INT_PTR CDlgBase::DialogProc_Internal( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_hwnd = hwndDlg;
	BOOL bHandled = FALSE;
	if( WM_COMMAND == uMsg )
	{
		//Command_Ex
		BOOL bHandled = FALSE;
		int iSize = m_Cmd_Ex_MsgMap.size();
		for( int i = 0; i < iSize; i ++ )
		{
			if( ( LOWORD( wParam ) == m_Cmd_Ex_MsgMap[ i ].m_iCmdID ) &&  ( HIWORD( wParam ) == m_Cmd_Ex_MsgMap[ i ].m_iNotifyCode ) )
			{
				COMMAND_HANDLER_EX pHandler = m_Cmd_Ex_MsgMap[ i ].m_pHandler;
				if( pHandler )
				{
					bHandled = TRUE;
					( this->*pHandler )( hwndDlg,( HWND )lParam, wParam, lParam );
				}
				break;
			}
		}

		//Command消息
		if( !bHandled )
		{
			iSize = m_Cmd_MsgMap.size();
			for( int i = 0; i < iSize; i ++ )
			{
				if( LOWORD( wParam ) == m_Cmd_MsgMap[ i ].m_iCmdID )
				{
					COMMAND_HANDLER pHandler = m_Cmd_MsgMap[ i ].m_pHandler;
					if( pHandler )
					{
						( this->*pHandler )( hwndDlg, wParam, lParam );
					}
					break;
				}
			}
		}
		
	}
	else if( WM_NOTIFY == uMsg )
	{
		//Notify 消息
		int iSize = m_Notify_MsgMap.size();
		for( int i = 0; i < iSize; i ++ )
		{
			LPNMHDR pHeader = ( LPNMHDR )lParam;
			NOTIFY_HANDLER pHandler = m_Notify_MsgMap[ i ].m_pHandler;
			if( pHeader->idFrom == m_Notify_MsgMap[ i ].m_iCtrlID )
			{
				if( pHeader->code == m_Notify_MsgMap[ i ].m_iNotifyCode )
				{
					if( pHandler )
					{
						return ( this->*pHandler )( hwndDlg, pHeader->hwndFrom, wParam, lParam );//NotifyHander是有返回结果的
					}
					break;
				}
			}
		}
	}
	else
	{
		//普通消息
		int iSize = m_WM_MsgMap.size();
		for( int i = 0; i < iSize; i ++ )
		{
			if( uMsg ==  m_WM_MsgMap[ i ].m_iMsgID )
			{
				MSG_HANDLER pHandler = m_WM_MsgMap[ i ].m_pHandler;
				if( pHandler )
				{
					( this->*pHandler )( hwndDlg, wParam, lParam );
				}
				break;
			}
		}
	}
	return FALSE;
}
