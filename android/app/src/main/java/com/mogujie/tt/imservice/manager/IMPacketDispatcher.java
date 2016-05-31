package com.mogujie.tt.imservice.manager;

import com.google.protobuf.CodedInputStream;
import com.mogujie.tt.protobuf.IMBaseDefine;
import com.mogujie.tt.protobuf.IMBuddy;
import com.mogujie.tt.protobuf.IMGroup;
import com.mogujie.tt.protobuf.IMLogin;
import com.mogujie.tt.protobuf.IMMessage;
import com.mogujie.tt.utils.Logger;

import java.io.IOException;

/**
 * yingmu
 * 消息分发中心，处理消息服务器返回的数据包
 * 1. decode  header与body的解析
 * 2. 分发
 */
public class IMPacketDispatcher {
	private static Logger logger = Logger.getLogger(IMPacketDispatcher.class);

    /**
     * @param commandId
     * @param buffer
     *
     * 有没有更加优雅的方式
     */
    public static void loginPacketDispatcher(int commandId,CodedInputStream buffer){
        try {
        switch (commandId) {
//            case IMBaseDefine.LoginCmdID.CID_LOGIN_RES_USERLOGIN_VALUE :
//                IMLogin.IMLoginRes  imLoginRes = IMLogin.IMLoginRes.parseFrom(buffer);
//                IMLoginManager.instance().onRepMsgServerLogin(imLoginRes);
//                return;

            case IMBaseDefine.LoginCmdID.CID_LOGIN_RES_LOGINOUT_VALUE:
                IMLogin.IMLogoutRsp imLogoutRsp = IMLogin.IMLogoutRsp.parseFrom(buffer);
                IMLoginManager.instance().onRepLoginOut(imLogoutRsp);
                return;

            case IMBaseDefine.LoginCmdID.CID_LOGIN_KICK_USER_VALUE:
                IMLogin.IMKickUser imKickUser = IMLogin.IMKickUser.parseFrom(buffer);
                IMLoginManager.instance().onKickout(imKickUser);
            }
        } catch (IOException e) {
            logger.e("loginPacketDispatcher# error,cid:%d",commandId);
        }
    }

    public static void buddyPacketDispatcher(int commandId,CodedInputStream buffer){
        try {
        switch (commandId) {
            case IMBaseDefine.BuddyListCmdID.CID_BUDDY_LIST_ALL_USER_RESPONSE_VALUE:
                    IMBuddy.IMAllUserRsp imAllUserRsp = IMBuddy.IMAllUserRsp.parseFrom(buffer);
                    IMContactManager.instance().onRepAllUsers(imAllUserRsp);
                return;

            case IMBaseDefine.BuddyListCmdID.CID_BUDDY_LIST_USER_INFO_RESPONSE_VALUE:
                   IMBuddy.IMUsersInfoRsp imUsersInfoRsp = IMBuddy.IMUsersInfoRsp.parseFrom(buffer);
                    IMContactManager.instance().onRepDetailUsers(imUsersInfoRsp);
                return;
            case IMBaseDefine.BuddyListCmdID.CID_BUDDY_LIST_RECENT_CONTACT_SESSION_RESPONSE_VALUE:
                IMBuddy.IMRecentContactSessionRsp recentContactSessionRsp = IMBuddy.IMRecentContactSessionRsp.parseFrom(buffer);
                IMSessionManager.instance().onRepRecentContacts(recentContactSessionRsp);
                return;

            case IMBaseDefine.BuddyListCmdID.CID_BUDDY_LIST_REMOVE_SESSION_RES_VALUE:
                IMBuddy.IMRemoveSessionRsp removeSessionRsp = IMBuddy.IMRemoveSessionRsp.parseFrom(buffer);
                    IMSessionManager.instance().onRepRemoveSession(removeSessionRsp);
                return;

            case IMBaseDefine.BuddyListCmdID.CID_BUDDY_LIST_PC_LOGIN_STATUS_NOTIFY_VALUE:
                IMBuddy.IMPCLoginStatusNotify statusNotify = IMBuddy.IMPCLoginStatusNotify.parseFrom(buffer);
                IMLoginManager.instance().onLoginStatusNotify(statusNotify);
                return;

            case IMBaseDefine.BuddyListCmdID.CID_BUDDY_LIST_DEPARTMENT_RESPONSE_VALUE:
                IMBuddy.IMDepartmentRsp departmentRsp = IMBuddy.IMDepartmentRsp.parseFrom(buffer);
                IMContactManager.instance().onRepDepartment(departmentRsp);
                return;

        }
        } catch (IOException e) {
            logger.e("buddyPacketDispatcher# error,cid:%d",commandId);
        }
    }

    public static void msgPacketDispatcher(int commandId, CodedInputStream buffer) {
        try {
            switch (commandId) {
                case IMBaseDefine.MessageCmdID.CID_MSG_DATA_ACK_VALUE:
                    // have some problem  todo
                    return;
                case IMBaseDefine.MessageCmdID.CID_MSG_LIST_RESPONSE_VALUE:
                    IMMessage.IMGetMsgListRsp rsp = IMMessage.IMGetMsgListRsp.parseFrom(buffer);
                    IMMessageManager.instance().onReqHistoryMsg(rsp);
                    return;

                case IMBaseDefine.MessageCmdID.CID_MSG_DATA_VALUE:
                    IMMessage.IMMsgData imMsgData = IMMessage.IMMsgData.parseFrom(buffer);
                    IMMessageManager.instance().onRecvMessage(imMsgData);
                    return;

                case IMBaseDefine.MessageCmdID.CID_MSG_READ_NOTIFY_VALUE:
                    IMMessage.IMMsgDataReadNotify readNotify = IMMessage.IMMsgDataReadNotify.parseFrom(buffer);
                    IMUnreadMsgManager.instance().onNotifyRead(readNotify);
                    return;
                case IMBaseDefine.MessageCmdID.CID_MSG_UNREAD_CNT_RESPONSE_VALUE:
                    IMMessage.IMUnreadMsgCntRsp unreadMsgCntRsp = IMMessage.IMUnreadMsgCntRsp.parseFrom(buffer);
                    IMUnreadMsgManager.instance().onRepUnreadMsgContactList(unreadMsgCntRsp);
                    return;

                case IMBaseDefine.MessageCmdID.CID_MSG_GET_BY_MSG_ID_RES_VALUE:
                    IMMessage.IMGetMsgByIdRsp getMsgByIdRsp = IMMessage.IMGetMsgByIdRsp.parseFrom(buffer);
                    IMMessageManager.instance().onReqMsgById(getMsgByIdRsp);
                    break;

                case IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_REQUEST_VALUE:
                    // 收到实时语音请求的
                    // 进入到确认接受通话页面
//                    IMUIHelper.openConfirmAudioActivity(getActivity(),currentUser.getSessionKey());
//                    getActivity().finish();
                    IMMessage.IMMsgData imMsgAData = IMMessage.IMMsgData.parseFrom(buffer);
                    IMMessageManager.instance().onRecvAMessage(imMsgAData);
                    break;
                case IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_RESPONSE_VALUE:
                    // udp_server 返回的UDP
                    IMMessage.IMAudioRsp audioRsp = IMMessage.IMAudioRsp.parseFrom(buffer);

                    break;
                case IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_DATA_VALUE:
                    IMMessage.IMAudioData audioData = IMMessage.IMAudioData.parseFrom(buffer);

                    break;

            }
        } catch (IOException e) {
            logger.e("msgPacketDispatcher# error,cid:%d", commandId);
        }
    }

    public static void groupPacketDispatcher(int commandId,CodedInputStream buffer){
        try {
            switch (commandId) {
//                case IMBaseDefine.GroupCmdID.CID_GROUP_CREATE_RESPONSE_VALUE:
//                    IMGroup.IMGroupCreateRsp groupCreateRsp = IMGroup.IMGroupCreateRsp.parseFrom(buffer);
//                    IMGroupManager.instance().onReqCreateTempGroup(groupCreateRsp);
//                    return;

                case IMBaseDefine.GroupCmdID.CID_GROUP_NORMAL_LIST_RESPONSE_VALUE:
                    IMGroup.IMNormalGroupListRsp normalGroupListRsp = IMGroup.IMNormalGroupListRsp.parseFrom(buffer);
                    IMGroupManager.instance().onRepNormalGroupList(normalGroupListRsp);
                    return;

                case IMBaseDefine.GroupCmdID.CID_GROUP_INFO_RESPONSE_VALUE:
                    IMGroup.IMGroupInfoListRsp groupInfoListRsp = IMGroup.IMGroupInfoListRsp.parseFrom(buffer);
                    IMGroupManager.instance().onRepGroupDetailInfo(groupInfoListRsp);
                    return;

//                case IMBaseDefine.GroupCmdID.CID_GROUP_CHANGE_MEMBER_RESPONSE_VALUE:
//                    IMGroup.IMGroupChangeMemberRsp groupChangeMemberRsp = IMGroup.IMGroupChangeMemberRsp.parseFrom(buffer);
//                    IMGroupManager.instance().onReqChangeGroupMember(groupChangeMemberRsp);
//                    return;

                case IMBaseDefine.GroupCmdID.CID_GROUP_CHANGE_MEMBER_NOTIFY_VALUE:
                    IMGroup.IMGroupChangeMemberNotify notify = IMGroup.IMGroupChangeMemberNotify.parseFrom(buffer);
                    IMGroupManager.instance().receiveGroupChangeMemberNotify(notify);
                case IMBaseDefine.GroupCmdID.CID_GROUP_SHIELD_GROUP_RESPONSE_VALUE:
                    //todo
                    return;
            }
        }catch(IOException e){
            logger.e("groupPacketDispatcher# error,cid:%d",commandId);
            }
        }
}
