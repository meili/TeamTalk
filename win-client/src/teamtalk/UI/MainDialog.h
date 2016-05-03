 /*******************************************************************************
 *  @file      MainDialog.h 2014\7\31 15:23:22 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     主窗口
 ******************************************************************************/

#ifndef MAINDIALOG_8BE12371_DCA1_4FC5_8E32_D74B82D2399B_H__
#define MAINDIALOG_8BE12371_DCA1_4FC5_8E32_D74B82D2399B_H__

#define WM_START_MOGUTALKINSTANCE WM_USER + 101

#include "GlobalDefine.h"
#include "Modules/ModuleObserver.h"
/******************************************************************************/
enum
{
	ICON_TRAY_LOGO = 0,
	ICON_TRAY_NEWMSG,
	ICON_TRAY_LEAVE,
	ICON_TRAY_OFFLINE,
	ICON_COUNT,
};

enum BalloonStyle
{
	BALLOON_WARNING,
	BALLOON_ERROR,
	BALLOON_INFO,
	BALLOON_NONE,
	BALLOON_USER,
};

#define WM_TRAYICON_NOTIFY	WM_USER+1002

const UInt16  TIMER_TRAYEMOT = 1;//系统托盘闪烁timer


class CNotifyIconData : public NOTIFYICONDATA
{
public:
	CNotifyIconData()
	{
		memset(this, 0, sizeof(NOTIFYICONDATA));
		cbSize = sizeof(NOTIFYICONDATA);
	}
};

/**
 * The class <code>MainDialog</code> 
 *
 */
class MainDialog final : public WindowImplBase
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    MainDialog();
    /**
     * Destructor
     */
    virtual ~MainDialog();
    //@}
	DUI_DECLARE_MESSAGE_MAP()
public:
	LPCTSTR GetWindowClassName() const;
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual void InitWindow();
	virtual void OnFinalMessage(HWND hWnd);
    virtual void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	void OnCheckUpdateSucc(WPARAM wparam, LPARAM lparam);
	void OnCheckUpdate(WPARAM wparam, LPARAM lparam);

private:
	void Initilize();
	void CreateTrayIcon();
	void LoadIcons();
	void UpdateLineStatus(UInt8 status);
	void SetTrayIconIndex(int nIndex);

	void OnWindowInitialized(TNotifyUI& msg);
	void OnClick(TNotifyUI& msg);
	void OnCopyData(IN COPYDATASTRUCT* pCopyData);
	void OnTextChanged(TNotifyUI& msg);
	void OnMenuClicked(IN const CString& itemName,IN const CString& strLparam);
	void OnTrayIconNotify(WPARAM wParam, LPARAM lParam);
    void OnHotkey(__in WPARAM wParam, __in LPARAM lParam);
	/**@name MKO*/
	//@{
public:
	void MKOForTcpClientModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	void MKOForLoginModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	void MKOForUserListModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	void MKOForSessionModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	//@}
	
	/**@name 托盘区图标相关操作*/
	//@{
	BOOL InstallIcon(LPCTSTR lpszToolTip, HICON hIcon, UINT nID, BOOL bReInstall = FALSE);
	BOOL RemoveIcon();
	BOOL SetTrayIcon(HICON hIcon);
	BOOL HideIcon(void);
	BOOL ShowIcon();
	BOOL IsIconInstalled();
	BOOL SetTrayTooltipText(LPCTSTR pszTooltipText);
	BOOL SetBalloonDetails(LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, BalloonStyle style = BALLOON_INFO, UINT nTimeout = 1000, HICON hUserIcon = NULL, BOOL bNoSound = FALSE);
	BOOL SetVersion(UINT uVersion);
	DWORD GetShellVersion(void);

    void _UpdateTotalUnReadMsgCount(void);//更新总的未读计数
    void _FreshMySignature(void);
	/**
	 * 开始系统托盘闪烁
	 *
	 * @return  void
	 * @exception there is no any exception to throw.
	 */	
	void StartNewMsgTrayEmot();
	/**
	* 停止系统托盘闪烁
	*
	* @return  void
	* @exception there is no any exception to throw.
	*/
	void StopNewMsgTrayEmot();
	//@}

private:
	CButtonUI*				m_pbtnSysConfig;
	CButtonUI*				m_pbtnOnlineStatus;
	CButtonUI*				m_pbtnMyFace;
	CButtonUI*				m_pbtnClose;
	CButtonUI*				m_pbtnMinMize;
	CTextUI*				m_ptxtUname;
    CTextUI*				m_pTextUnreadMsgCount;//总的未读计数
    CEditUI*	            m_pEditSignature;//个性签名


	HICON                   m_hIcons[ICON_COUNT];                       //Icon对象数组	
	CNotifyIconData			m_niData;
	BOOL					m_bInstalled;
	BOOL					m_bHidden;
	DWORD					m_dwShellVersion;
};
/******************************************************************************/
#endif// MAINDIALOG_8BE12371_DCA1_4FC5_8E32_D74B82D2399B_H__


