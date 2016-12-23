package com.mogujie.tt.imservice.manager;

import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;

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
 *
 *  判断 如果两个客户端外网IP相同， 不需要NAT打洞
 *      如果有一台机器 在NAT外部 不需要打洞
 *
 *      如果NAT打洞失败，需要转发;
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

        // 发送指令给udp_server
        SocketAddress serverAddress = new InetSocketAddress("123.57.71.215", 8132);
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
        int cid = IMBaseDefine.MessageCmdID.CID_MSG_DATA_VALUE;//CID_MSG_AUDIO_UDP_REQUEST_VALUE;

        final MessageEntity messageEntity  = msgEntity;
        // 发送到服务器 // 需要回复的 new Packetlistener
        imSocketManager.sendRequest(msgData,sid,cid,new Packetlistener(getTimeoutTolerance(messageEntity)) {
            @Override
            public void onSuccess(Object response) {
                try {
                    IMMessage.IMMsgDataAck imMsgDataAck = IMMessage.IMMsgDataAck.parseFrom((CodedInputStream)response);
                    logger.i("chat#onAckSendedMsg#messageUDPReceived");
                    if(imMsgDataAck.getMsgId() <=0){
                        throw  new RuntimeException("Msg ack error,cause by msgId <=0");
                    }
                    messageEntity.setStatus(MessageConstant.MSG_SUCCESS);
                    messageEntity.setMsgId(imMsgDataAck.getMsgId());
                    /**主键ID已经存在，直接替换*/
                    //dbInterface.insertOrUpdateMessage(messageEntity);
                    /**更新sessionEntity lastMsgId问题*/
                    //sessionManager.updateSession(messageEntity);
                    // 发送，发送成功 收到这个表明发送成功 ACK_SEND_MESSAGE_OK处理中会出错
                    //triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_OK,messageEntity));
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            @Override
            public void onFaild() {
                logger.i("chat#onAckSendedMsg#onFaild");

                messageEntity.setStatus(MessageConstant.MSG_FAILURE);
                //dbInterface.insertOrUpdateMessage(messageEntity);
                triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_FAILURE,messageEntity));
            }
            @Override
            public void onTimeout() {
                logger.i("chat#onAckSendedMsg#onTimeout");

                messageEntity.setStatus(MessageConstant.MSG_FAILURE);
                //dbInterface.insertOrUpdateMessage(messageEntity);
                triggerEvent(new MessageEvent(MessageEvent.Event.ACK_SEND_MESSAGE_TIME_OUT,messageEntity));
            }
        });
    }

    /**
     * 发送音频数据
     * @param sendContent
     */
    public void SendRealAudioData(byte[] sendContent, int size){

        // 1024 的 数组 没有填满；
        final byte[] buf = new byte[size];
//        String forSend = ("audio_data_" + SequenceNumberMaker.getInstance().make());
        System.arraycopy(buf, 0, sendContent, 0, size);

        IMMessage.IMAudioData audioReq = null;
//        try {
            audioReq = IMMessage.IMAudioData.newBuilder()
                    .setFromUserId(1)
                    .setSeqNum(SequenceNumberMaker.getInstance().make())   // 0打洞数据
                    .setClientType(IMBaseDefine.ClientType.CLIENT_TYPE_ANDROID)
//                    .setMsgData(ByteString.copyFrom(forSend,"utf-8"))
                    .setMsgData(ByteString.copyFrom(buf))
                    .build();
//        } catch (UnsupportedEncodingException e) {
//            e.printStackTrace();
//        }

        int sid = IMBaseDefine.ServiceID.SID_MSG_VALUE;
        int cid = IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_DATA_VALUE; // 音频数据
        imSocketUDPManager.sendUDPRequest(audioReq,sid,cid,null,m_serverAddress);

    }

    private UserEntity m_loginUser;
    private SocketAddress m_serverAddress;

    /**
     * 发送消息，最终的状态情况
     * MessageManager下面的拆分
     * 应该是自己发的信息，所以msgId为0
     * 这个地方用DB id作为主键
     */
    public void SendAudioData(UserEntity loginUser,ByteString sendContent,SocketAddress serverAddress, int seqNum){
        m_loginUser = loginUser;
        m_serverAddress = serverAddress;

        // 发送情况下 msg_id 都是0
        // 服务端是从1开始计数的
//        if(!SequenceNumberMaker.getInstance().isFailure(audioRsp.getMsgId())){
//            throw new RuntimeException("#sendMessage# msgId is wrong,cause by 0!");
//        }

//        IMBaseDefine.MsgType msgType = Java2ProtoBuf.getProtoMsgType(msgEntity.getMsgType());
//        byte[] sendContent = msgEntity.getSendContent();
//        logger.d("channel#messageUDPReceived send poll_ok 1111" + loginUser);
        // loginUser.getId().intValue() == null
        // 发送给UDP_client的
        IMMessage.IMAudioData audioReq = IMMessage.IMAudioData.newBuilder()
                .setFromUserId(1)
                .setSeqNum(seqNum)   // 0打洞数据
                .setClientType(IMBaseDefine.ClientType.CLIENT_TYPE_ANDROID)
                .setMsgData(sendContent)
                .build();
//        logger.d("channel#messageUDPReceived send poll_ok 2222");

        // 登入UDP服务器，带上要连的房间号，两个客户端同一房间号相通	(语音请求,msg加一个消息类型)

        int sid = IMBaseDefine.ServiceID.SID_MSG_VALUE;
        int cid = IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_DATA_VALUE; // 音频数据
        imSocketUDPManager.sendUDPRequest(audioReq,sid,cid,null,serverAddress);

        logger.d("channel#messageUDPReceived send poll_ok");

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
                .setToRoomId(msgEntity.getFromId() + msgEntity.getToId()) // 房间号暂定为请求人的id
                .setMsgId(0)        // 0 加入 1 退出
                .setCreateTime(msgEntity.getCreated())
                .setMsgType(msgType)    // 消息类型  MSG_TYPE_SINGLE_AUDIO_MEET 语音
                .setClientType(IMBaseDefine.ClientType.CLIENT_TYPE_ANDROID)
                .setLocalIp(getLocalIpAddress())
                .build();

        int sid = IMBaseDefine.ServiceID.SID_MSG_VALUE;
        // 登入UDP服务器，带上要连的房间号，两个客户端同一房间号相通	(语音请求,msg加一个消息类型)
        int cid = IMBaseDefine.MessageCmdID.CID_MSG_AUDIO_UDP_REQUEST_VALUE;

        final MessageEntity messageEntity  = msgEntity;
        // 发送到服务器 // 需要回复的 new Packetlistener （服务端回复）
        imSocketUDPManager.sendUDPRequest(audioReq,sid,cid,new Packetlistener(6000) {
            @Override
            public void onSuccess(Object response) {
                //
            }
            @Override
            public void onFaild() {
                messageEntity.setStatus(MessageConstant.MSG_FAILURE);
            }
            @Override
            public void onTimeout() {
                messageEntity.setStatus(MessageConstant.MSG_FAILURE);
                // 超时重传一次？
            }
        },serverAddress);
    }


    private String getLocalIpAddress() {
//        if(!GetNetworkType(ctx).equals("WIFI")){
//            return "";
//        }
        WifiManager wifiManager = (WifiManager) ctx.getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        // 获取32位整型IP地址
        int ipAddress = wifiInfo.getIpAddress();

        //返回整型地址转换成“*.*.*.*”地址
        return String.format("%d.%d.%d.%d",
                (ipAddress & 0xff), (ipAddress >> 8 & 0xff),
                (ipAddress >> 16 & 0xff), (ipAddress >> 24 & 0xff));
    }

//    public String GetNetworkType(Context ctx)
//    {
//        String strNetworkType = "";
//
//        NetworkInfo networkInfo = (ConnectivityManager) ctx.getSystemService(Context.CONNECTIVITY_SERVICE).getActiveNetworkInfo();
//        if (networkInfo != null && networkInfo.isConnected())
//        {
//            if (networkInfo.getType() == ConnectivityManager.TYPE_WIFI)
//            {
//                strNetworkType = "WIFI";
//            }
//            else if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE)
//            {
//                String _strSubTypeName = networkInfo.getSubtypeName();
//
////                Log.e("cocos2d-x", "Network getSubtypeName : " + _strSubTypeName);
//
//                // TD-SCDMA   networkType is 17
//                int networkType = networkInfo.getSubtype();
//                switch (networkType) {
//                    case TelephonyManager.NETWORK_TYPE_GPRS:
//                    case TelephonyManager.NETWORK_TYPE_EDGE:
//                    case TelephonyManager.NETWORK_TYPE_CDMA:
//                    case TelephonyManager.NETWORK_TYPE_1xRTT:
//                    case TelephonyManager.NETWORK_TYPE_IDEN: //api<8 : replace by 11
//                        strNetworkType = "2G";
//                        break;
//                    case TelephonyManager.NETWORK_TYPE_UMTS:
//                    case TelephonyManager.NETWORK_TYPE_EVDO_0:
//                    case TelephonyManager.NETWORK_TYPE_EVDO_A:
//                    case TelephonyManager.NETWORK_TYPE_HSDPA:
//                    case TelephonyManager.NETWORK_TYPE_HSUPA:
//                    case TelephonyManager.NETWORK_TYPE_HSPA:
//                    case TelephonyManager.NETWORK_TYPE_EVDO_B: //api<9 : replace by 14
//                    case TelephonyManager.NETWORK_TYPE_EHRPD:  //api<11 : replace by 12
//                    case TelephonyManager.NETWORK_TYPE_HSPAP:  //api<13 : replace by 15
//                        strNetworkType = "3G";
//                        break;
//                    case TelephonyManager.NETWORK_TYPE_LTE:    //api<11 : replace by 13
//                        strNetworkType = "4G";
//                        break;
//                    default:
//                        // http://baike.baidu.com/item/TD-SCDMA 中国移动 联通 电信 三种3G制式
//                        if (_strSubTypeName.equalsIgnoreCase("TD-SCDMA") || _strSubTypeName.equalsIgnoreCase("WCDMA") || _strSubTypeName.equalsIgnoreCase("CDMA2000"))
//                        {
//                            strNetworkType = "3G";
//                        }
//                        else
//                        {
//                            strNetworkType = _strSubTypeName;
//                        }
//
//                        break;
//                }
//
////                Log.e("cocos2d-x", "Network getSubtype : " + Integer.valueOf(networkType).toString());
//            }
//        }
//
////        Log.e("cocos2d-x", "Network Type : " + strNetworkType);
//
//        return strNetworkType;
//    }
}
