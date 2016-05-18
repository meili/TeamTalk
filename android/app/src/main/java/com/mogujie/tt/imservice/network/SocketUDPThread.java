package com.mogujie.tt.imservice.network;

import com.google.protobuf.GeneratedMessageLite;
import com.mogujie.tt.config.SysConstant;
import com.mogujie.tt.protobuf.base.DataBuffer;
import com.mogujie.tt.protobuf.base.Header;
import com.mogujie.tt.utils.Logger;

import org.jboss.netty.bootstrap.ConnectionlessBootstrap;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.channel.SimpleChannelUpstreamHandler;
import org.jboss.netty.channel.socket.nio.NioDatagramChannelFactory;

import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.util.concurrent.Executors;

public class SocketUDPThread extends Thread {
	private ConnectionlessBootstrap bootstrap = null;
	private ChannelFactory channelFactory = null;
//	private ChannelFuture channelFuture = null;
	private Channel channel = null;
	private String strHost = null;
	private int nPort = 0;
	private static Logger logger = Logger.getLogger(SocketUDPThread.class);

	public SocketUDPThread(String strHost, int nPort, SimpleChannelUpstreamHandler handler) {
		this.strHost = strHost;
		this.nPort = nPort;
		init(handler);
	}

	@Override
	public void run() {
	}

	private void init(final SimpleChannelUpstreamHandler handler) { // SimpleChannelHandler
		// only one IO thread
		//
//		channelFactory = new NioClientSocketChannelFactory(
		channelFactory = new NioDatagramChannelFactory(Executors.newCachedThreadPool());

		bootstrap = new ConnectionlessBootstrap(channelFactory);

		bootstrap.setPipelineFactory(new ChannelPipelineFactory() {
			@Override
			public ChannelPipeline getPipeline() throws Exception {
				ChannelPipeline pipeline = Channels.pipeline();

				pipeline.addLast("handler", handler);

				return pipeline;//Channels.pipeline(new UdpEventHandler());
			}
		});
//		int port = 8312;
		SocketAddress serverAddress = new InetSocketAddress(nPort);
		channel = bootstrap.bind(serverAddress);
	}

    /**
     * @param requset
     * @param header
     * @return
     */
    public boolean sendUDPRequest(GeneratedMessageLite requset,Header header, String sendHost, int sendPort){
		logger.e("#sendUDPRequest#channel!111111111111");

		DataBuffer headerBuffer = header.encode();
        DataBuffer bodyBuffer = new DataBuffer();
        int bodySize = requset.getSerializedSize();
        bodyBuffer.writeBytes(requset.toByteArray());

        DataBuffer buffer = new DataBuffer(SysConstant.PROTOCOL_HEADER_LENGTH  + bodySize);
        buffer.writeDataBuffer(headerBuffer);
        buffer.writeDataBuffer(bodyBuffer);

		SocketAddress serverAddress = new InetSocketAddress(sendHost, sendPort);

//		IMMessage.IMAudioReq audiodata = IMMessage.IMAudioReq.newBuilder()
////                .setFromUserId(msgEntity.getFromId())
//                .build();
//		channel.write(audiodata, serverAddress);
		logger.e("#sendUDPRequest#channel!");

		channel.write(buffer.getOrignalBuffer(), serverAddress);

		return true;
    }

}
