 /*******************************************************************************
 *  @file      UIIMList.h 2014\8\12 9:07:38 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef UIIMLIST_548A8B8B_30ED_4E85_8BB4_32FB9EC52FE6_H__
#define UIIMLIST_548A8B8B_30ED_4E85_8BB4_32FB9EC52FE6_H__

#include "GlobalDefine.h"
#include "Modules/ModuleObserver.h"
#include "UIIMListCommonDefine.h"

using namespace DuiLib;

class IMListItemInfo
{
public:
	IMListItemInfo();
	~IMListItemInfo();

	bool folder;
	bool empty;
	CDuiString id;
	CDuiString avatarPath;
	CDuiString nickName;
	CDuiString description;
};

/******************************************************************************/

/**
 * The class <code>UIIMList</code> 
 *
 */
class UIIMList : public CListUI
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	UIIMList(CPaintManagerUI& paint_manager);
    /**
     * Destructor
     */
    virtual ~UIIMList();
    //@}

public:
	enum { SCROLL_TIMERID = 10 };
	bool Add(CControlUI* pControl);
	bool AddAt(CControlUI* pControl, int iIndex);
	bool Remove(CControlUI* pControl);
	bool RemoveAt(int iIndex);
	void RemoveAll();
	void DoEvent(TEventUI& event);
	Node* GetRoot();
	Node* AddNode(const IMListItemInfo& item, Node* parent = NULL, int index = 0);
	bool RemoveNode(Node* node);
	void SetChildVisible(Node* node, bool visible);
	bool CanExpand(Node* node) const;
	virtual void DoInit();

public:
	/**@name sId和item对应相关*/
	//@{
	BOOL IsExistSId(IN const std::string& sId);
	/**
	 * 获取列表里所有项目的SID，给创建群使用
	 *
	 * @return  std::vector<std::string>
	 * @exception there is no any exception to throw.
	 */	
	std::vector<std::string> GetAllItemsSId();
	Node* GetItemBySId(IN const std::string& sId);
	//@}

	/**
	 * 先set一个Item路径，再addnod
	 *
	 * @param   CString IitemXml
	 * @return  void
	 * @exception there is no any exception to throw.
	 */	
	void SetIitemXmlFile(CString IitemXml);
	void SetItemNormalHeight(const int nIMListItemNormalHeight);
	Node* GetParentIdForAddNode(IN CDuiString sId);
	void ExpandAll();
    static void SetTextUICount(IN CControlUI* pCTextUI, IN UInt32 nCount);

	/**@name MKO*/
	//@{
public:
	void MKOForUserListModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	void MKOForTcpClientModuleCallback(const std::string& keyId, MKO_TUPLE_PARAM mkoParam);
	//@}

private:
	void _freshAvatarBySId(std::shared_ptr<void> pParam);
	void _updateCount(IN const UInt32 nCount,OUT CString& strContent);
public:
	Node*			root_node_;
	LONG			delay_deltaY_;
	DWORD			delay_number_;
	DWORD			delay_left_;
	CDuiRect		text_padding_;
	int				level_text_start_pos_;
	CDuiString		level_expand_image_;
	CDuiString		level_collapse_image_;

	CPaintManagerUI& paint_manager_;
	CDialogBuilder m_dlgBuilder;

private:
	CString		m_IitemXmlFilePath;
	int			m_IMListItemNormalHeight = 60;
};
/******************************************************************************/
#endif// UIIMLIST_548A8B8B_30ED_4E85_8BB4_32FB9EC52FE6_H__
