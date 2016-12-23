package com.mogujie.tt.imservice.manager;

import com.google.protobuf.CodedInputStream;
import com.google.protobuf.GeneratedMessageLite;
import com.mogujie.tt.config.SysConstant;
import com.mogujie.tt.imservice.callback.ListenerQueue;
import com.mogujie.tt.imservice.callback.Packetlistener;
import com.mogujie.tt.imservice.event.SocketEvent;
import com.mogujie.tt.imservice.network.SocketUDPThread;
import com.mogujie.tt.imservice.network.UDPServerHandler;
import com.mogujie.tt.protobuf.IMBaseDefine;
import com.mogujie.tt.protobuf.base.DataBuffer;
import com.mogujie.tt.protobuf.base.DefaultHeader;
import com.mogujie.tt.utils.Logger;

import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.buffer.ChannelBufferInputStream;
import org.jboss.netty.channel.MessageEvent;

import java.net.SocketAddress;

import de.greenrobot.event.EventBus;

/**
 * @author : yingmu on 14-12-30.
 * @email : yingmu@mogujie.com.
 * <p/>
 * 业务层面:
 * 绑定端口，接收消息，发送消息
 *
 */
public class IMSocketUDPManager extends IMManager {

    private Logger logger = Logger.getLogger(IMSocketUDPManager.class);
    private static IMSocketUDPManager inst = new IMSocketUDPManager();

    public static IMSocketUDPManager instance() {
        return inst;
    }

    public IMSocketUDPManager() {
        logger.d("login#creating IMSocketManager");
    }

    private ListenerQueue listenerQueue = ListenerQueue.instance();

    // 请求消息服务器地址
    //private AsyncHttpClient client = new AsyncHttpClient();

    /**
     * 底层socket
     */
    private SocketUDPThread msgUDPServerThread;

    /**
     * 自身状态
     */
    private SocketEvent socketStatus = SocketEvent.NONE;

    /**
     * 获取Msg地址，等待链接
     */
    @Override
    public void doOnStart() {
        socketStatus = SocketEvent.NONE;
        listenerQueue.onStart();
    }


    //todo check
    @Override
    public void reset() {
        socketStatus = SocketEvent.NONE;
    }

    /**
     * 实现自身的事件驱动
     *
     * @param event
     */
    public void triggerEvent(SocketEvent event) {
        EventBus.getDefault().postSticky(event);
    }

    /**
     * -------------------------------功能方法--------------------------------------
     */


    /**
     *
     * @param requset
     * @param sid
     * @param cid
     * @param packetlistener 不需要回复的这个传参 null
     */
    public void sendUDPRequest(GeneratedMessageLite requset, int sid, int cid, Packetlistener packetlistener,SocketAddress serverAddress) {
        int seqNo = 0;
        try {
            //组装包头 header
            com.mogujie.tt.protobuf.base.Header header = new DefaultHeader(sid, cid);
            int bodySize = requset.getSerializedSize();
            header.setLength(SysConstant.PROTOCOL_HEADER_LENGTH + bodySize);
            //seqNo = header.getSeqnum();
            listenerQueue.push(seqNo, packetlistener);
            if(msgUDPServerThread!=null){
                logger.d("#sendRequest#channel is not open!msgUDPServerThread send");
                boolean sendRes = msgUDPServerThread.send_UDP_request(requset, header, serverAddress);
            } else {
                logger.d("#sendUDPRequest#msgUDPServerThread is null!");
            }
        } catch (Exception e) {
            if (packetlistener != null) {
                packetlistener.onFaild();
            }
            listenerQueue.pop(seqNo);
            logger.e("#sendRequest#channel is close!" + e.toString());
        }
    }

    /**
     * 接收到数据
     *
     * @param channelBuffer
     */
    public void packetUDPDispatch(ChannelBuffer channelBuffer,MessageEvent e) {
//        logger.d("channel#messageUDPReceived#packetUDPDispatch#1");
        DataBuffer buffer = new DataBuffer(channelBuffer);
        com.mogujie.tt.protobuf.base.Header header = new com.mogujie.tt.protobuf.base.Header();
//        logger.d("channel#messageUDPReceived#packetUDPDispatch#2");
        header.decode(buffer);
        /** buffer 的指针位于body的地方*/
        int commandId = header.getCommandId();
        int serviceId = header.getServiceId();
        int seqNo = header.getSeqnum();
//        buffer.readDataBuffer();

        CodedInputStream codedInputStream = CodedInputStream.newInstance(new ChannelBufferInputStream(buffer.getOrignalBuffer()));

        // UDP 打洞成功的包确认 如果是回复的消息，调用到onsuccess
        Packetlistener listener = listenerQueue.pop(seqNo);
        if (listener != null) { // 需要回复的消息收到返回调用onSuccess
            listener.onSuccess(codedInputStream);
            logger.d("channel#messageUDPReceived#onsuccess");
            return;
        }

        // todo eric make it a table
        // 抽象 父类执行
        switch (serviceId) {
            case IMBaseDefine.ServiceID.SID_LOGIN_VALUE: // 登录消息
                IMPacketDispatcher.loginPacketDispatcher(commandId, codedInputStream);
                break;
            case IMBaseDefine.ServiceID.SID_BUDDY_LIST_VALUE:// 好友消息
                IMPacketDispatcher.buddyPacketDispatcher(commandId, codedInputStream);
                break;
            case IMBaseDefine.ServiceID.SID_MSG_VALUE:  // 发送消息
                IMPacketDispatcher.msgPacketDispatcher(commandId, codedInputStream,e);
                break;
            case IMBaseDefine.ServiceID.SID_GROUP_VALUE:
                IMPacketDispatcher.groupPacketDispatcher(commandId, codedInputStream);
                break;
            default:
                logger.e("packet#unhandled serviceId:%d, commandId:%d", serviceId,
                        commandId);
                break;
        }
    }

    public void reqServerAddrs() {
        logger.e("#reqServerAddrs#start SocketUDPThread");

        // 绑定本地的UDP端口
        msgUDPServerThread = new SocketUDPThread("127.0.0.1", 8132, new UDPServerHandler());
        msgUDPServerThread.start();
    }

}
