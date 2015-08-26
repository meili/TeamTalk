/*******************************************************************************
 *  @file      SessionDialog.h 2015\3\10 10:19:29 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef SESSIONDIALOG_E6052CEB_BBCA_4E54_8D60_1AED7DCC8F4C_H__
#define SESSIONDIALOG_E6052CEB_BBCA_4E54_8D60_1AED7DCC8F4C_H__

#include "GlobalDefine.h"
#include "UIEAUserTreelist.h"

class SessionLayout;
class MessageEntity;
class SessionDialog : public WindowImplBase
{
public:

	SessionDialog(const std::string& sId);
	virtual ~SessionDialog();

public:
	LPCTSTR GetWindowClassName() const;	
	virtual void InitWindow();
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void Notify(TNotifyUI& msg);
	void OnWindowInitialized(TNotifyUI& msg);
	void OnMenuClicked(IN const CString& itemName, IN const CString& strLparam);
	virtual LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	virtual LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	void UpdateRunTimeMsg(void);
	void DoDisplayHistoryMsgToIE(IN std::vector<MessageEntity>& msgList, IN const BOOL scrollBottom);

	//语音播放动画操作
	BOOL StopPlayingAnimate(std::string& sAudioPlayingID);
	BOOL StartPlayingAnimate(std::string sAudioPlayingID);

	/**@name MKO*/
	//@{
	void MKOForSessionModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	void MKOForSysConfigModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	void MKOForTcpClientModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	void MKOForLoginModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	void MKOForUserListModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
    void MKOForScreenCaptureModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	//@}

private:
	void _ShakeWindow();
	void _FreshAvatar();
    void _FreshSignature(void);

public:
	CButtonUI*			m_pBtnAvatar;
	CButtonUI*			m_pBtnMax;
	CButtonUI*			m_pBtnRestore;
	CTextUI*			m_pWritingStatus;
	CTextUI*			m_pTxtName;
    CEditUI*	        m_pEditSignature;//对方的个性签名

	SessionLayout*		m_pSessionLayout;
	std::string			m_sId;		//会话Id
};

#endif // SESSIONDIALOG_E6052CEB_BBCA_4E54_8D60_1AED7DCC8F4C_H__