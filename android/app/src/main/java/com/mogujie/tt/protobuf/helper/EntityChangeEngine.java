package com.mogujie.tt.protobuf.helper;

import android.text.TextUtils;

import com.mogujie.tt.DB.entity.MessageEntity;
import com.mogujie.tt.DB.entity.SessionEntity;
import com.mogujie.tt.config.DBConstant;
import com.mogujie.tt.imservice.entity.MsgAnalyzeEngine;
import com.mogujie.tt.imservice.entity.UnreadEntity;

/**
 * @author : yingmu on 15-1-5.
 * @email : yingmu@mogujie.com.
 */
public class EntityChangeEngine {

    public static SessionEntity getSessionEntity(MessageEntity msg){
        SessionEntity sessionEntity = new SessionEntity();

        // [图文消息] [图片] [语音]
        sessionEntity.setLatestMsgData(msg.getMessageDisplay());
        sessionEntity.setUpdated(msg.getUpdated());
        sessionEntity.setCreated(msg.getUpdated());
        sessionEntity.setLatestMsgId(msg.getMsgId());
        //sessionEntity.setPeerId(msg.getFromId());
        sessionEntity.setTalkId(msg.getFromId());
        sessionEntity.setPeerType(msg.getSessionType());
        sessionEntity.setLatestMsgType(msg.getMsgType());
        return  sessionEntity;
    }


    //ked 2016-06-03 用于收到未读消息时， 生成会话
    //TODO：缺少session update\create字段。。。
    public static SessionEntity getSessionEntity(UnreadEntity ue){
        SessionEntity sessionEntity = new SessionEntity();

        // [图文消息] [图片] [语音]
        int msgType = ue.getLaststMsgType();

        String content  = ue.getLatestMsgData();
        String desMessage = new String(com.mogujie.tt.Security.getInstance().DecryptMsg(content));
        // 判断具体的类型是什么
        if(msgType == DBConstant.MSG_TYPE_GROUP_TEXT ||
                msgType ==DBConstant.MSG_TYPE_SINGLE_TEXT){
            desMessage =  MsgAnalyzeEngine.analyzeMessageDisplay(desMessage);
        }
        sessionEntity.setLatestMsgData( desMessage );

        //sessionEntity.setUpdated(msg.getUpdated());
        //sessionEntity.setCreated(msg.getUpdated());
        sessionEntity.setLatestMsgId(ue.getLaststMsgId());
        sessionEntity.setPeerId(ue.getPeerId());
        sessionEntity.setTalkId(ue.getLatestMsgFromUserId());
        sessionEntity.setPeerType(ue.getSessionType());
        sessionEntity.setLatestMsgType(msgType);

        return  sessionEntity;
    }

    // todo enum
    // 组建与解析统一地方，方便以后维护
    public static String getSessionKey(int peerId,int sessionType){
        String sessionKey = sessionType + "_" + peerId;
        return sessionKey;
    }

    public static String[] spiltSessionKey(String sessionKey){
        if(TextUtils.isEmpty(sessionKey)){
            throw new IllegalArgumentException("spiltSessionKey error,cause by empty sessionKey");
        }
        String[] sessionInfo = sessionKey.split("_",2);
        return sessionInfo;
    }
}
