/*******************************************************************************
 *  @file      IUserListModule.h 2014\8\6 15:25:06 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     组织架构相关模块
 ******************************************************************************/

#ifndef IUSERLISTMODULE_FDBD79FF_2F63_4E0A_8265_A001E2EB5182_H__
#define IUSERLISTMODULE_FDBD79FF_2F63_4E0A_8265_A001E2EB5182_H__

#include "GlobalDefine.h"
#include "Modules/ModuleDll.h"
#include "Modules/IMiscModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/ModuleBase.h"
#include "Modules/IModuleInterface.h"
#include "utility/utilStrCodingAPI.h"
#include "ProtocolBuffer/IM.BaseDefine.pb.h"
#include <string>
#include <list>
/******************************************************************************/
#define		PREFIX_GRAY_AVATAR		_T("gray_")

NAMESPACE_BEGIN(module)
const std::string MODULE_USERLIST_PREFIX = "userlist";
//KEYID
const std::string KEY_USERLIST_UPDATE_DEPARTMENTLIST = MODULE_USERLIST_PREFIX + "DeparmentList"; 		//成功获取到组织架构信息
const std::string KEY_USERLIST_UPDATE_RECENTLIST = MODULE_USERLIST_PREFIX + "RecentList";			//成功获取最近联系人
const std::string KEY_USERLIST_UPDATE_NEWUSESADDED = MODULE_USERLIST_PREFIX + "NewuserAdded";			//新用户更新
const std::string KEY_USERLIST_DOWNAVATAR_SUCC = MODULE_USERLIST_PREFIX + "DownavatarSucc";		//头像下载成功通知
const std::string KEY_USERLIST_ALLUSERLINESTATE = MODULE_USERLIST_PREFIX + "AllUserLineState";		//列表所有用户在线状态通知
const std::string KEY_USERLIST_USERLINESTATE = MODULE_USERLIST_PREFIX + "UserLineState";			//单个用户在线状态通知
const std::string KEY_USERLIST_USERSIGNINFO_CHANGED = MODULE_USERLIST_PREFIX + "UserSignInfoChanged";//用户个性签名更变通知

/**
* The class <code>部门信息定义</code>
*
*/
struct DepartmentEntity
{
	std::string				dId;				//部门ID
	UInt32					priority;			//排序优先级
	CString					name;				//部门名称
	std::string				parentDepartId;		//上级部门名称	
	UInt8					status;				//部门状态  0:正常 1:删除
	UInt32					latestUpdateTime;	//最后更新时间
	std::list<std::string>	members;			//成员列表
};
typedef std::map<std::string, DepartmentEntity> DepartmentMap;

class MODULE_CLASS UserInfoEntity
{
public:
	CString getRealName()const;
	/**
	* 根据在线状态获取头像
	*
	* @return  std::string 头像路径
	* @exception there is no any exception to throw.
	*/
	std::string getAvatarPath()const;
	/**
	 * 获取纯头像，没有在线状态的区分
	 *
	 * @return  std::string 头像路径
	 * @exception there is no any exception to throw.
	 */	
	std::string getAvatarPathWithoutOnlineState()const;
	BOOL isOnlne()const;

public:
	UInt8           onlineState = IM::BaseDefine::USER_STATUS_OFFLINE;            //用户在线状态  	USER_STATUS_ONLINE 	= 1,USER_STATUS_OFFLINE	= 2,USER_STATUS_LEAVE	= 3,
	UInt32			avatarHashcode = 0;		//根据avatarUrl计算的hash值
	UInt32			updated = 0;			//最后一次消息更新时间
	UInt32			gender = 1; 			// 用户性别,男：1 女：2 人妖/外星人：0
	CString         csName;                 //用户名
	CString         csNickName;             //用户昵称
	std::string     avatarUrl;              //用户头像地址
	std::string     avatarLocalPath;        //头像下载成功后的存储路径
	std::string     avatarGrayLocalPath;    //灰度处理之后的头像存储路径
	std::string     sId;                    //公司员工id
	std::string		dId;					//部门id
	std::string		department;				//部门名称
	std::string		email;					//邮箱
	std::string     user_domain;			//用户花名拼音
	std::string		telephone;				//电话
    std::string     signature;              //个性签名
	UInt32			status;					//0:在职  1:离职
};
typedef std::map<std::string, UserInfoEntity>    UserInfoEntityMap;
typedef std::vector<std::string>			UserInfoEntityVec;

/**
 * The class <code>组织架构相关模块</code> 
 *
 */
class MODULE_API IUserListModule : public module::ModuleBase
								  ,public module::IPduPacketParse
{
public:
	/**
	* 用户模块初始化处理（加载本地用户数据）
	*
	* @return  
	* @exception there is no any exception to throw.
	*/
	virtual BOOL startup() = 0;
	/**
	* 所有部门信息
	*
	* @return  const module::DepartmentMap&
	* @exception there is no any exception to throw.
	*/
	virtual const module::DepartmentMap& getAllDepartments() = 0;
	/**
	* 获取所有人的信息
	*
	* @param   module::UserInfoEntityMap & MapUsers
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void getAllUsersInfo(OUT module::UserInfoEntityMap& MapUsers)const = 0;
	/**
	* 根据用户ID获取个人信息
	*
	* @param   IN std::string sid
	* @param   OUT module::UserInfoEntity & userInfo
	* @return  BOOL
	* @exception there is no any exception to throw.
	*/
	virtual BOOL getUserInfoBySId(IN std::string sid, OUT module::UserInfoEntity& userInfo) = 0;
	/**
	* 清空所有人列表
	*
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void removeAllListInfo() = 0;
	/**
	* 获取自己的信息
	*
	* @return  UInt8
	* @exception there is no any exception to throw.
	*/
	virtual UInt8 getMyLineStatus()= 0;

	/**
	* 获取某个人的在线状态
	*
	* @param   IN const std::string & sId
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void tcpGetUserOnlieStatus(IN const std::string& sId) = 0;
	/**
	* 获取一批人的在线列表，如果列表人数过多，要分批获取
	*
	* @param   const module::VecUserInfoEntity & VecId
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void tcpGetUserOnlieStatus(IN const module::UserInfoEntityVec& VecId) = 0;
	/**
	* tcp获取个人信息
	*
	* @param   IN const std::string & sId
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void tcpGetUserInfo(IN const std::string& sId) = 0;
	/**
	* tcp获取某一批人的信息
	*
	* @param   IN const module::VecUserInfoEntity & VecUnKnowUserInfo
	* @return  void
	* @exception there is no any exception to throw.
	*/
    virtual void tcpChangeMySignInfo(IN const std::string sSignInfo) = 0;//修改自己的个性签名

	virtual void tcpGetUsersInfo(IN const module::UserInfoEntityVec& VecUnKnowUserInfo) = 0;
	/**
	* 获取缺省的个人头像
	*
	* @return  CString
	* @exception there is no any exception to throw.
	*/
	virtual CString getDefaultAvatartPath() = 0;
	/**
	* 获取自己的信息
	*
	* @return BOOL
	* @exception there is no any exception to throw.
	*/
	virtual BOOL getMyInfo(OUT module::UserInfoEntity& myInfo) = 0;

	/**
	 * 根据缩写或者全拼获取搜索的人的名称
	 *
	 * @param   IN const CString & sShortName
	 * @param   OUT std::vector<CString> & nameList
	 * @return  void
	 * @exception there is no any exception to throw.
	 */
	virtual void getSearchUserNameListByShortName(IN const CString& sShortName, OUT	module::UserInfoEntityVec& nameList) = 0;
	virtual void getSearchDepartmentsByShortName(IN const CString& sShortName, OUT	std::vector<std::string>& departmentList) = 0;

	/**
	 * 这个接口目前就是给离线消息、创建登陆者信息用的,创建不考虑替换，如果存在sId了就失败处理
	 *
	 * @param   const std::string & sId
	 * @param   const module::UserInfoEntity & info
	 * @return  BOOL
	 * @exception there is no any exception to throw.
	 */	
	virtual BOOL createUserInfo(IN const UserInfoEntity& info) = 0;

	virtual std::string randomGetUser(void) = 0;//随机获取一个异性ID
};

MODULE_API IUserListModule* getUserListModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// IUSERLISTMODULE_FDBD79FF_2F63_4E0A_8265_A001E2EB5182_H__
