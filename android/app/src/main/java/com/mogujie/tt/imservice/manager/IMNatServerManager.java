package com.mogujie.tt.imservice.manager;

import com.google.protobuf.ByteString;
import com.google.protobuf.CodedInputStream;
import com.mogujie.tt.DB.entity.MessageEntity;
import com.mogujie.tt.DB.entity.UserEntity;
import com.mogujie.tt.config.DBConstant;
import com.mogujie.tt.config.MessageConstant;
import com.mogujie.tt.imservice.callback.Packetlistener;
import com.mogujie.tt.imservice.entity.TextMessage;
import com.mogujie.tt.imservice.event.MessageEvent;
import com.mogujie.tt.imservice.support.SequenceNumberMaker;
import com.mogujie.tt.protobuf.IMBaseDefine;
import com.mogujie.tt.protobuf.IMMessage;
import com.mogujie.tt.protobuf.helper.Java2ProtoBuf;
import com.mogujie.tt.utils.Logger;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;

import de.greenrobot.event.EventBus;

/**
 * @author : xieqq on 16-05-10.
 * @email : 342418923@qq.com
 *
 * 业务层面:
 *  音视频 UDP打洞
 *  传文件是TCP打洞( TCP的暂不考虑 TCP的可用UDP实现，加上丢包重传 再重组的 机制)
 *
 *  clientA Message发送实时语音请求(指令)-->
 *
 *  clientA 绑定本地UDP端口，给NAT_SERVER服务端UDP端口发登录指令
 *      开启接收线程 收到B上线，给clinet A打洞，等待回复 （ 3秒重发一次10秒超时)
 *
 *      --> 收到回复后告诉对方准备好 iReady+1
 *
 *
 *     一分钟给服务端UDP发送一心跳，服务端维持住登陆状态 (服务端是否不需要维持，下次重连再请求)
 *          (超过一分钟收不到客户端指令的，从服务端列表退出)
 *          开始实时传送后，让服务端退出；（下次断线再重连）
 *
 *
 *  clientB Message server接收
 *  clientB 绑定本地UDP端口, 给NAT_SERVER服务端UDP端口发登录指令（服务端把B上线通知A）
 *      开启接收线程
 *      --> 1、给clientA UDP 打洞 等待回复；（ 6秒重发一次10秒超时)
 *
 *      --> 收到回复后告诉对方准备好 iReady+1
 *
 *  iReady == 2开始实时传送
 *
 */
public class IMNatServerManager extends IMManager {

    private Logger logger = Logger.getLogger(IMNatServerManager.class);
    private static IMNatServerManager inst = new IMNatServerManager();
    public static IMNatServerManager instance() {
        return inst;
    }

    private IMSocketManager imSocketManager = IMSocketManager.instance();
    private IMSocketUDPManager imSocketUDPManager = IMSocketUDPManager.instance();

    @Override
    public void doOnStart() {

    }

    @Override
    public void reset() {

    }

    //
    public void SendCommand(TextMessage textMessage){
        // 发送消息给msg_server
        logger.i("chat#text#textMessage");
        textMessage.setStatus(MessageConstant.MSG_SENDING);
//        long pkId =  DBInterface.instance().insertOrUpdateMessage(textMessage);
//        sessionManager.updateSession(textMessage);
        sendMessage(textMessage); // 发给msg_server 的

        SocketAddress serverAddress = new InetSocketAddress("123.57.71.215", 8132);
        // 发送指令给udp_server
        sendUDPMessage(textMessage,serverAddress);
    }

//    /**
//     * 发送消息给 msg_server
//     * 2. push到adapter中
//     * 3. 等待ack,更新页面
//     * */
//    private void sendText(TextMessage textMessage) {
//        logger.i("chat#text#textMessage");
//        textMessage.setStatus(MessageConstant.MSG_SENDING);
////        long pkId =  DBInterface.instance().insertOrUpdateMessage(textMessage);
////        sessionManager.updateSession(textMessage);
//
//        sendMessage(textMessage);
//        sendUDPMessage(textMessage); // 发送消息给 msg_server
//    }

    // 消息发送超时时间爱你设定
    // todo eric, after testing ok, make it a longer value
    private final long TIMEOUT_MILLISECONDS = 6 * 1000;
    private final long IMAGE_TIMEOUT_MILLISECONDS = 4 * 60 * 1000;


    private long getTimeoutTolerance(MessageEntity msg) {
        switch (msg.getDisplayType()){
            case DBConstant.SHOW_IMAGE_TYPE:
                return IMAGE_TIMEOUT_MILLISECONDS;
            default:break;
        }
        return TIMEOUT_MILLISECONDS;
    }

    /**
     * 自身的事件驱动
     * @param event
     */
    public void triggerEvent(Object event) {
        EventBus.getDefault().post(event);
    }

    // 发送给msg_server的
    public void sendMessage(MessageEntity msgEntity) {
        logger.d("chat_audio#sendMessage, msg:%s", msgEntity);
        // 发送情况下 msg_id 都是0
        // 服务端是从1开始计数的
        if(!SequenceNumberMaker.getInstance().isFailure(msgEntity.getMsgId())){
            throw new RuntimeException("#sendMessage_audio# msgId is wrong,cause by 0!");
        }

        IMBaseDefine.MsgType msgType = Java2ProtoBuf.getProtoMsgType(msgEntity.getMsgType());
        byte[] sendContent = msgEntity.getSendContent();

        // 消息数据
        IMMessage.IMMsgData msgData = IMMessage.IMMsgData.newBuilder()
                .setFromUserId(msgEntity.getFromId())
                .setToSessionId(msgEntity.getToId())
                .setMsgId(0)
                .setCreateTime(msgEntity.getCreated())
                .setMsgType(msgType)
                .setMsgData(ByteString.copyFrom(sendContent))  // 这个点要特别注意 todo ByteString.copyFrom
                .build();
        int sid = IMBaseDefine.ServiceID.SID_MSG_VALUE;
        int cid = IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_REQUEST_VALUE;//CID_MSG_DATA_VALUE;

        final MessageEntity messageEntity  = msgEntity;
        // 发送到服务器 // 需要回复的 new Packetlistener
        imSocketManager.sendRequest(msgData,sid,cid,new Packetlistener(getTimeoutTolerance(messageEntity)) {
            @Override
            public void onSuccess(Object response) {
                try {
                    IMMessage.IMMsgDataAck imMsgDataAck = IMMessage.IMMsgDataAck.parseFrom((CodedInputStream)response);
                    logger.i("chat#onAckSendedMsg");
                    if(imMsgDataAck.getMsgId() <=0){
                        throw  new RuntimeException("Msg ack error,cause by msgId <=0");
                    }
                    messageEntity.setStatus(MessageConstant.MSG_SUCCESS);
                    messageEntity.setMsgId(imMsgDataAck.getMsgId());
                    /**主键ID已经存在，直接替换*/
                    //dbInterface.insertOrUpdateMessage(messageEntity);
                    /**更新sessionEntity lastMsgId问题*/
                    //sessionManager.updateSession(messageEntity);
                    // 发送，发送成功 收到这个表明发送成功
                    triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_OK,messageEntity));
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            @Override
            public void onFaild() {
                messageEntity.setStatus(MessageConstant.MSG_FAILURE);
                //dbInterface.insertOrUpdateMessage(messageEntity);
                triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_FAILURE,messageEntity));
            }
            @Override
            public void onTimeout() {
                messageEntity.setStatus(MessageConstant.MSG_FAILURE);
                //dbInterface.insertOrUpdateMessage(messageEntity);
                triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_TIME_OUT,messageEntity));
            }
        });
    }

    /**
     * 发送消息，最终的状态情况
     * MessageManager下面的拆分
     * 应该是自己发的信息，所以msgId为0
     * 这个地方用DB id作为主键
     */
    public void SendAudioData(UserEntity loginUser,ByteString sendContent,SocketAddress serverAddress){
        // 发送情况下 msg_id 都是0
        // 服务端是从1开始计数的
//        if(!SequenceNumberMaker.getInstance().isFailure(audioRsp.getMsgId())){
//            throw new RuntimeException("#sendMessage# msgId is wrong,cause by 0!");
//        }

//        IMBaseDefine.MsgType msgType = Java2ProtoBuf.getProtoMsgType(msgEntity.getMsgType());
//        byte[] sendContent = msgEntity.getSendContent();

        // 发送给UDP_client的
        IMMessage.IMAudioData audioReq = IMMessage.IMAudioData.newBuilder()
                .setFromUserId(loginUser.getId().intValue())
                .setSeqNum(0)   // 0打洞数据
                .setClientType(IMBaseDefine.ClientType.CLIENT_TYPE_ANDROID)
                .setMsgData(sendContent)
                .build();

        // 登入UDP服务器，带上要连的房间号，两个客户端同一房间号相通	(语音请求,msg加一个消息类型)

        int sid = IMBaseDefine.ServiceID.SID_MSG_VALUE;
        int cid = IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_DATA_VALUE; // 音频数据

//        final MessageEntity messageEntity  = msgEntity;

        // 发送到服务器 // 需要回复的 new Packetlistener （服务端回复）
        imSocketUDPManager.sendUDPRequest(audioReq,sid,cid,new Packetlistener(12000) {
            @Override
            public void onSuccess(Object response) {
                // 打洞成功
                // 开始发送音频数据
//                try {
//                    IMMessage.IMMsgDataAck imMsgDataAck = IMMessage.IMMsgDataAck.parseFrom((CodedInputStream)response);
//                    logger.i("chat#onAckSendedMsg");
//                    if(imMsgDataAck.getMsgId() <=0){
//                        throw  new RuntimeException("Msg ack error,cause by msgId <=0");
//                    }
//                    messageEntity.setStatus(MessageConstant.MSG_SUCCESS);
//                    messageEntity.setMsgId(imMsgDataAck.getMsgId());

                    // 指令不存库，UDP不用管发送成功失败

                    /**主键ID已经存在，直接替换*/
//                    dbInterface.insertOrUpdateMessage(messageEntity);
                    /**更新sessionEntity lastMsgId问题*/
//                    sessionManager.updateSession(messageEntity);
                    // 发送，发送成功 收到这个表明发送成功
//                    triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_OK,messageEntity));
//                } catch (IOException e) {
//                    e.printStackTrace();
//                }
            }
            @Override
            public void onFaild() {
//                messageEntity.setStatus(MessageConstant.MSG_FAILURE);
//                dbInterface.insertOrUpdateMessage(messageEntity);
//                triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_FAILURE,messageEntity));
            }
            @Override
            public void onTimeout() {
                // 超时
//                messageEntity.setStatus(MessageConstant.MSG_FAILURE);
//                dbInterface.insertOrUpdateMessage(messageEntity);
//                triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_TIME_OUT,messageEntity));
            }
        },serverAddress);
    }

    /**
     * 发送消息，最终的状态情况
     * MessageManager下面的拆分
     * 应该是自己发的信息，所以msgId为0
     * 这个地方用DB id作为主键
     */
    public void sendUDPMessage(MessageEntity msgEntity,SocketAddress serverAddress) {
        logger.d("chat_audio#sendUDPMessage, msg:%s", msgEntity);
        // 发送情况下 msg_id 都是0
        // 服务端是从1开始计数的
        if(!SequenceNumberMaker.getInstance().isFailure(msgEntity.getMsgId())){
            throw new RuntimeException("#sendUDPMessage# msgId is wrong,cause by 0!");
        }

        IMBaseDefine.MsgType msgType = Java2ProtoBuf.getProtoMsgType(msgEntity.getMsgType());
        byte[] sendContent = msgEntity.getSendContent();

        // 发送给UDP SERVER的
        IMMessage.IMAudioReq audioReq = IMMessage.IMAudioReq.newBuilder()
                .setFromUserId(msgEntity.getFromId())
                .setToRoomId(msgEntity.getFromId()) // 房间号暂定为请求人的id
                .setMsgId(0)        // 0 加入 1 退出
                .setCreateTime(msgEntity.getCreated())
                .setMsgType(msgType)    // 消息类型  MSG_TYPE_SINGLE_AUDIO_MEET 语音
                .setClientType(IMBaseDefine.ClientType.CLIENT_TYPE_ANDROID)
                .build();

//        IMMessage.IMAudioReq audiodata = IMMessage.IMAudioReq.newBuilder()
//                .setFromUserId(msgEntity.getFromId())
//                .build();
        int sid = IMBaseDefine.ServiceID.SID_MSG_VALUE;
        // 登入UDP服务器，带上要连的房间号，两个客户端同一房间号相通	(语音请求,msg加一个消息类型)
        int cid = IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_REQUEST_VALUE;

        final MessageEntity messageEntity  = msgEntity;
        // 发送到服务器 // 需要回复的 new Packetlistener （服务端回复）
        imSocketUDPManager.sendUDPRequest(audioReq,sid,cid,new Packetlistener(6000) {
            @Override
            public void onSuccess(Object response) {
//                try {
//                    IMMessage.IMMsgDataAck imMsgDataAck = IMMessage.IMMsgDataAck.parseFrom((CodedInputStream)response);
//                    logger.i("chat#onAckSendedMsg");
//                    if(imMsgDataAck.getMsgId() <=0){
//                        throw  new RuntimeException("Msg ack error,cause by msgId <=0");
//                    }
//                    messageEntity.setStatus(MessageConstant.MSG_SUCCESS);
//                    messageEntity.setMsgId(imMsgDataAck.getMsgId());
//
//                    // 指令不存库，UDP不用管发送成功失败
//
//                    /**主键ID已经存在，直接替换*/
////                    dbInterface.insertOrUpdateMessage(messageEntity);
//                    /**更新sessionEntity lastMsgId问题*/
////                    sessionManager.updateSession(messageEntity);
//                    // 发送，发送成功 收到这个表明发送成功
////                    triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_OK,messageEntity));
//                } catch (IOException e) {
//                    e.printStackTrace();
//                }
            }
            @Override
            public void onFaild() {
                messageEntity.setStatus(MessageConstant.MSG_FAILURE);
//                dbInterface.insertOrUpdateMessage(messageEntity);
//                triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_FAILURE,messageEntity));
            }
            @Override
            public void onTimeout() {
                messageEntity.setStatus(MessageConstant.MSG_FAILURE);
//                dbInterface.insertOrUpdateMessage(messageEntity);
//                triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_TIME_OUT,messageEntity));
            }
        },serverAddress);
    }
}
