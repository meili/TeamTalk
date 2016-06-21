package com.mogujie.tt.imservice.manager;

import com.google.protobuf.ByteString;
import com.google.protobuf.CodedInputStream;
import com.mogujie.tt.DB.entity.UserEntity;
import com.mogujie.tt.app.IMApplication;
import com.mogujie.tt.protobuf.IMBaseDefine;
import com.mogujie.tt.protobuf.IMBuddy;
import com.mogujie.tt.protobuf.IMGroup;
import com.mogujie.tt.protobuf.IMLogin;
import com.mogujie.tt.protobuf.IMMessage;
import com.mogujie.tt.utils.Logger;

import org.jboss.netty.channel.MessageEvent;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;

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

    //
//    public static String inet_ntoa(long add) {
//        return ((add & 0xff000000) >> 24) + "." + ((add & 0xff0000) >> 16)
//                + "." + ((add & 0xff00) >> 8) + "." + ((add & 0xff));
//    }

//    public static String inet_ntoa2(long add) {
//        return ((add & 0xff)) + "." + ((add & 0xff00) >> 8)
//                + "." + ((add & 0xff0000) >> 16) + "." + ((add & 0xff000000) >> 24);
//
////        private static byte[] toLH(int n) {
////            byte[] b = new byte[4];
////            b[0] = (byte) (n & 0xff);
////            b[1] = (byte) (n >> 8 & 0xff);
////            b[2] = (byte) (n >> 16 & 0xff);
////            b[3] = (byte) (n >> 24 & 0xff);
////            return b;
////        }
//    }

    public static void msgPacketDispatcher(int commandId, CodedInputStream buffer,MessageEvent msgE) {
//        logger.d("channel#messageUDPReceived#msgPacketDispatcher commandId:%d", commandId);
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
                    logger.d("channel#messageUDPReceived#msgPacketDispatcher CID_MSG_AUDIO_UDP_REQUEST_VALUE");

                    // 收到实时语音请求的(好友发来的语音请求)
                    // 进入到确认接受通话页面
//                    IMUIHelper.openConfirmAudioActivity(getActivity(),currentUser.getSessionKey());
//                    getActivity().finish();
                    // 换成了 CID_MSG_DATA_VALUE
//                    IMMessage.IMMsgData imMsgAData = IMMessage.IMMsgData.parseFrom(buffer);
//                    IMMessageManager.instance().onRecvAMessage(imMsgAData);
                    break;
                case IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_RESPONSE_VALUE:
                    logger.d("channel#messageUDPReceived#msgPacketDispatcher CID_MSG_AUDIO_UDP_RESPONSE_VALUE __ OK");
                    // udp_server 返回的UDP请求包
                    IMMessage.IMAudioRsp audioRsp = IMMessage.IMAudioRsp.parseFrom(buffer);
                    logger.d("channel#messageUDPReceived# %d_%s_%d" ,
                            audioRsp.getFromUserId(),
                            audioRsp.getUserList().getIp(),
                            audioRsp.getUserList().getPort()
                            );
//                    inet_ntoa2(audioRsp.getUserList(0).getIp())
                    UserEntity loginUser = IMLoginManager.instance().getLoginInfo();
//                    if (audioRsp.getUserList(0).getUserId() == loginUser.getId()) {
//                        logger.d("channel#messageUDPReceived_loginuser" + loginUser.getId());
//                        return;
//                    }
//                    if(audioRsp.getUserList(0).getUserId() != loginUser.getId())
                    {
                        logger.d("channel#messageUDPReceived send poll_%s_%d",
                                audioRsp.getUserList().getIp(),
                                audioRsp.getUserList().getPort());

                        // 让连谁
//                        String sendContent =new String(com.mogujie.tt.Security.getInstance().EncryptMsg("poll"));
                        String sendContent = "poll"; // sendContent.getBytes("utf-8")
                        // id ip port
                        IMApplication.connNid = audioRsp.getUserList().getUserId();
                        IMApplication.connstrIP = audioRsp.getUserList().getIp();
                        IMApplication.connNport = audioRsp.getUserList().getPort();
                        SocketAddress serverAddress = new InetSocketAddress(IMApplication.connstrIP, IMApplication.connNport);

                        // 局域网直连发送为啥不能？
//                         发送打洞数据 (几秒重发一次直到成功)
                        IMNatServerManager.instance().SendAudioData(
                                loginUser,
                                ByteString.copyFrom(sendContent,"utf-8"),
                                serverAddress, 0);
                    }
                    break;
                case IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_DATA_VALUE:
//                    SpeekerToast.show(, "1111", Toast.LENGTH_SHORT);
//                    logger.d("channel#messageUDPReceived#CID_MSG_AUDIO_UDP_DATA_VALUE");
                    IMMessage.IMAudioData audioData = IMMessage.IMAudioData.parseFrom(buffer);
                    logger.d("channel#messageUDPReceived#audioData" + audioData.getSeqNum());
//                    // 音频数据
//                    IMMessageManager.instance().onRecvAudioData(audioData);
                    if(audioData.getSeqNum() == 0){
                        // 打洞的包  // 把（id ip port存起来）根据ID找到IP、PORT
//                        IMNatServerManager.instance().SendAudioData(audioRsp,
//                                loginUser,
//                                null,
//                                audioRsp.getUserList(0).getIp(),
//                                audioRsp.getUserList(0).getPort());
                        // 打洞请求，发送回复 会不会服务器的UDP包比client先到
                        String sendContent = audioData.getMsgData().toString("utf-8");

                        logger.d("channel#messageUDPReceived# receive poll ++" + sendContent);

                        String forSend = (sendContent+ "_back");
                        if(sendContent.equals("poll_back") || sendContent.equals("poll")){
                            logger.d("channel#messageUDPReceived# receive poll__" + sendContent);
                            // 如果收到回复 // 启动开始发送音频数据的服务
                            IMNatServerManager.instance().SendAudioData(
                                    IMLoginManager.instance().getLoginInfo(),
                                    ByteString.copyFrom(forSend,"utf-8"),//.getBytes("utf-8"),
                                    msgE.getRemoteAddress(), 0); // 得到发送者回复过去
                        } else {
                            // 开启音频发送服务
                            // 类似把SpeexEncoder filewriter 改成 SendAudioData()
                            IMAudioManager.instance().startIMAudioSend();
                        }
                    } else {
                        // 音频数据播放
                        // SpeexDecoder // 解码 AudioTRack实时播放
                        IMMessageManager.instance().onRecvAMessage(audioData);
//                        String sendContent = audioData.getMsgData().toString("utf-8");
//                        logger.d("channel#messageUDPReceived# play_" + sendContent);

                    }
                    break;

            }
        } catch (IOException e) {
            logger.e("msgPacketDispatcher# error,cid:%d" + e.toString(), commandId);
        }
    }
    public static void msgPacketDispatcher(int commandId, CodedInputStream buffer) {
        msgPacketDispatcher(commandId,buffer,null);
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
