package com.mogujie.tt.imservice.entity;

import com.mogujie.tt.DB.entity.PeerEntity;
import com.mogujie.tt.DB.entity.UserEntity;
import com.mogujie.tt.config.DBConstant;
import com.mogujie.tt.DB.entity.MessageEntity;
import com.mogujie.tt.config.MessageConstant;
import com.mogujie.tt.imservice.support.SequenceNumberMaker;

import java.io.Serializable;
import java.io.UnsupportedEncodingException;

/**
 * @author : yingmu on 14-12-31.
 * @email : yingmu@mogujie.com.
 */
public class TextMessage extends MessageEntity implements Serializable {

     public TextMessage(){
         msgId = SequenceNumberMaker.getInstance().makelocalUniqueMsgId();
     }

     private TextMessage(MessageEntity entity){
         /**父类的id*/
         id =  entity.getId();
         msgId  = entity.getMsgId();
         fromId = entity.getFromId();
         toId   = entity.getToId();
         sessionKey = entity.getSessionKey();
         content=entity.getContent();
         msgType=entity.getMsgType();
         displayType=entity.getDisplayType();
         status = entity.getStatus();
         created = entity.getCreated();
         updated = entity.getUpdated();
     }

     public static TextMessage parseFromNet(MessageEntity entity){
         TextMessage textMessage = new TextMessage(entity);
         textMessage.setStatus(MessageConstant.MSG_SUCCESS);
         textMessage.setDisplayType(DBConstant.SHOW_ORIGIN_TEXT_TYPE);
         return textMessage;
     }

    public static TextMessage parseFromDB(MessageEntity entity){
        if(entity.getDisplayType()!=DBConstant.SHOW_ORIGIN_TEXT_TYPE){
            throw new RuntimeException("#TextMessage# parseFromDB,not SHOW_ORIGIN_TEXT_TYPE");
        }
        TextMessage textMessage = new TextMessage(entity);
        return textMessage;
    }

    public static TextMessage buildForSend(String content,UserEntity fromUser,PeerEntity peerEntity){
        TextMessage textMessage = new TextMessage();
        int nowTime = (int) (System.currentTimeMillis() / 1000);
        textMessage.setFromId(fromUser.getPeerId());
        textMessage.setToId(peerEntity.getPeerId());
        textMessage.setUpdated(nowTime);
        textMessage.setCreated(nowTime);
        textMessage.setDisplayType(DBConstant.SHOW_ORIGIN_TEXT_TYPE);
        textMessage.setGIfEmo(true);
        int peerType = peerEntity.getType();
        // 群消息，还是单个消息
        int msgType = peerType == DBConstant.SESSION_TYPE_GROUP ? DBConstant.MSG_TYPE_GROUP_TEXT
                : DBConstant.MSG_TYPE_SINGLE_TEXT;
        textMessage.setMsgType(msgType);
        textMessage.setStatus(MessageConstant.MSG_SENDING);
        // 内容的设定
        textMessage.setContent(content);
        textMessage.buildSessionKey(true);
        return textMessage;
    }

    public static TextMessage buildForCommandSend(int ntype,UserEntity fromUser,PeerEntity peerEntity){
        TextMessage textMessage = new TextMessage();
        int nowTime = (int) (System.currentTimeMillis() / 1000);
        textMessage.setFromId(fromUser.getPeerId());
        textMessage.setToId(peerEntity.getPeerId());
        textMessage.setUpdated(nowTime);
        textMessage.setCreated(nowTime);
        textMessage.setDisplayType(DBConstant.SHOW_ORIGIN_TEXT_TYPE);
        textMessage.setGIfEmo(true);
        int peerType = peerEntity.getType();

        // 单人语音聊天
        int msgType = DBConstant.MSG_TYPE_SINGLE_AUDIO_MEET;
        switch (ntype){
            case 1:
                // 群消息，还是单个消息
                msgType = (peerType == DBConstant.SESSION_TYPE_GROUP ? DBConstant.MSG_TYPE_GROUP_AUDIO_MEET
                        : DBConstant.MSG_TYPE_SINGLE_AUDIO_MEET);
                break;
        }

        textMessage.setMsgType(msgType);
        textMessage.setStatus(MessageConstant.MSG_SENDING);
        // 内容的设定
        textMessage.setContent(String.valueOf(ntype));
        textMessage.buildSessionKey(true);
        return textMessage;
    }

    /**
     * Not-null value.
     * DB的时候需要
     */
    @Override
    public String getContent() {
        return content;
    }

    @Override
    public byte[] getSendContent() {
        try {
            /** 加密*/
            String sendContent =new String(com.mogujie.tt.Security.getInstance().EncryptMsg(content));
            return sendContent.getBytes("utf-8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return null;
    }
}
