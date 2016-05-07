/*******************************************************************************
 *  @file      FileTransferDialog.h 2014\8\29 11:33:35 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     文件传输列表窗口
 ******************************************************************************/

#ifndef FILETRANSFERDIALOG_624E545F_4732_4817_913B_C929D5B87C9D_H__
#define FILETRANSFERDIALOG_624E545F_4732_4817_913B_C929D5B87C9D_H__

#include "DuiLib/UIlib.h"
#include "GlobalDefine.h"
#include "Modules/ModuleObserver.h"
/******************************************************************************/
using namespace DuiLib;

/**
 * The class <code>FileTransferDialog</code> 
 *
 */
class TransferFileEntity;
class FileTransferDialog : public WindowImplBase
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    FileTransferDialog();
    /**
     * Destructor
     */
    virtual ~FileTransferDialog();
    //@}
	LPCTSTR GetWindowClassName() const;
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual void OnFinalMessage(HWND hWnd);
	DUI_DECLARE_MESSAGE_MAP()

	/**@name MKO*/
	//@{
public:
	void MKOForFileTransferModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	//@}

public:
	void AddTransderItemToList(IN std::string& id);
	CListContainerElementUI* AddTransderItemToList(IN TransferFileEntity fileInfo);
	CListContainerElementUI* GetTransferItemById(std::string& id);

private:
	void OnWindowInitialized(TNotifyUI& msg);
	void OnClick(TNotifyUI& msg);

	void _UpdateItemButtons(IN CListContainerElementUI* pListElement
		, IN bool bShowbtnOpenFile
		, IN bool bShowbtnOpenFolder
		, IN bool bShowbtnAccpet
		, IN bool bShowbtnRefuse
		, IN bool bShowbtnCancel);
	void _UpdateFileTransferState(IN CListContainerElementUI* pListElement,IN const std::string& keyId);
	CListContainerElementUI* _GetItemBySid(IN std::string sid);
public:
	CListUI*					m_plstFileTransfer;
	CProgressUI*				m_pprsTransProgress;

};
/******************************************************************************/
#endif// FILETRANSFERDIALOG_624E545F_4732_4817_913B_C929D5B87C9D_H__
