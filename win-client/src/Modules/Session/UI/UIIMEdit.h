 /*******************************************************************************
 *  @file      UIIMEdit.h 2014\8\19 13:18:10 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef UIIMEDIT_3DC89058_72C4_4225_A6EB_0F39D182913F_H__
#define UIIMEDIT_3DC89058_72C4_4225_A6EB_0F39D182913F_H__


#include "DuiLib/UIlib.h"
using namespace DuiLib;
#include "GlobalDefine.h"


#define UIIMEdit_MSGTYPE_TEXTCHANGED (_T("UIIMEdit_TEXT_Changed"))


/******************************************************************************/

/**
 * The class <code>UIIMEdit</code> 
 *
 */
namespace
{
	const CString CS_SPLIT_CODE_START = _T("&$#@~^@[{:");
	const CString CS_SPLIT_CODE_END = _T(":}]&$~@#@");
}
struct ST_picData
{
	UINT32	nPos;
	CString strLocalPicPath;
	CString strNetPicPath;
};

class MixedMsg
{
public:
	MixedMsg();
	BOOL SetNetWorkPicPath(IN CString strLocalPicPath, IN CString strNetPicPath);
	BOOL SucceedToGetAllNetWorkPic();
	CString MakeMixedLocalMSG();
	CString MakeMixedNetWorkMSG();
	BOOL IsPureTextMsg();
	static CString AddPicTeg2Pic(IN CString picPath);
    void ReplaceReturnKey(void);
public:
	CString					m_strTextData;					//文字
	std::vector<ST_picData>	m_picDataVec;				//图片所在文字中的位置，图片的本地路径；图片的网络路径
private:
	UINT32					m_nSetNetWorkPathSuccTime;	//成功获得的图片的次数
};


class UIIMEdit :public CRichEditUI
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    UIIMEdit();
    /**
     * Destructor
     */
    virtual ~UIIMEdit();
    //@}

public:
	virtual void DoInit();
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
	BOOL GetContent(OUT	MixedMsg& mixMsg);
	virtual LPVOID GetInterface(LPCTSTR pstrName);
	virtual LPCTSTR GetClass() const;
	virtual bool OnTxViewChanged();
	int	GetObjectPos();
	void InsertImage(BSTR bstrFileName, SIZE size, BOOL isGif);
	HRESULT GetNewStorage(LPSTORAGE* ppStg);
	void	GetObjectInfo(IRichEditOle *pIRichEditOle);
	void ReleaseAllGif();
	BOOL GetPicPosAndPathbyOrder(IN UInt32 nOrder,OUT UInt32& nPos,OUT CString& path );

private:
	void _ImEditPaste();
	BOOL _SaveFile(IN HBITMAP hbitmap, OUT CString& strFilePath);
	HBITMAP _LoadAnImage(IN CString filePath);
	
private:
	CString                     m_strImagePath;
};
/******************************************************************************/
#endif// UIIMEDIT_3DC89058_72C4_4225_A6EB_0F39D182913F_H__
