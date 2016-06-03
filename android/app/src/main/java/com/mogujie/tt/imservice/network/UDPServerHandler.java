package com.mogujie.tt.imservice.network;

import com.mogujie.tt.imservice.manager.IMSocketUDPManager;
import com.mogujie.tt.utils.Logger;

import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelUpstreamHandler;

public class UDPServerHandler extends SimpleChannelUpstreamHandler {

    private Logger logger = Logger.getLogger(UDPServerHandler.class);

	@Override
	public void messageReceived(ChannelHandlerContext ctx, MessageEvent e)
			throws Exception {
		super.messageReceived(ctx, e);
        logger.d("channel#messageUDPReceived");

        // 重置AlarmManager的时间
        ChannelBuffer channelBuffer = (ChannelBuffer) e.getMessage();
//        SocketAddress remote = e.getRemoteAddress();
//        remote.
        if(null!=channelBuffer) // 接收到的消息处理
            IMSocketUDPManager.instance().packetUDPDispatch(channelBuffer,e);
        else {
            logger.d("channel#messageUDPReceived#channelBuffer is null");

        }
        // MessageEvent中通过调用getRemoteAddress()方法获得对端的SocketAddress 地址。
        //        e.getChannel().write("", e.getRemoteAddress());

    }

}
