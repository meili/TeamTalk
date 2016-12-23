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
		SocketAddress serverAddress = new InetSocketAddress(nPort);
		channel = bootstrap.bind(serverAddress);
	}

	/**
	 * @param requset
	 * @param header
	 * @return
	 */
	public boolean send_UDP_request(GeneratedMessageLite requset,Header header, final SocketAddress serverAddress){

		DataBuffer headerBuffer = header.encode();
		DataBuffer bodyBuffer = new DataBuffer();
		int bodySize = requset.getSerializedSize();
		bodyBuffer.writeBytes(requset.toByteArray());

		final DataBuffer buffer = new DataBuffer(SysConstant.PROTOCOL_HEADER_LENGTH  + bodySize);
		buffer.writeDataBuffer(headerBuffer); 	// 头 包括：sid, cid
		buffer.writeDataBuffer(bodyBuffer);		// body
		channel.write(buffer.getOrignalBuffer(), serverAddress);

//
////		DataBuffer buffer = new DataBuffer(channelBuffer);
//		com.mogujie.tt.protobuf.base.Header header123 = new com.mogujie.tt.protobuf.base.Header();
//		buffer.resetReaderIndex();
////        logger.d("channel#messageUDPReceived#packetUDPDispatch#2");
//		header123.decode(buffer);
////		buffer.readDataBuffer();
//		CodedInputStream codedInputStream = CodedInputStream.newInstance(new ChannelBufferInputStream(buffer.getOrignalBuffer()));
//
//		try{
//			IMMessage.IMAudioData audioData = IMMessage.IMAudioData.parseFrom(codedInputStream);
//		} catch (IOException e) {
//			logger.e("send_UDP_request# error,cid:%d" + e.toString());
//		}

// 		// 以下的发送不成功
//		new Thread(){
//			@Override
//			public void run()
//			{
//				byte[] req = new byte[buffer.getOrignalBuffer().readableBytes()];
//				buffer.getOrignalBuffer().readBytes(req);
//				 //把网络访问的代码放在这里
//				InetSocketAddress inetAddr = (InetSocketAddress) serverAddress;
//				try {
//					logger.d("#sendUDPRequest#sendPacket" + inetAddr.getAddress() + "_" + inetAddr.getPort());
//
//					// 创建发送方的套接字，IP默认为本地，端口号随机
//					DatagramSocket sendSocket = new DatagramSocket();
//					DatagramPacket sendPacket = new DatagramPacket(req,
//							buffer.getOrignalBuffer().readableBytes(),
//							inetAddr.getAddress(),
//							inetAddr.getPort());
//
//					sendSocket.send(sendPacket);
//					sendSocket.close();
//				} catch (SocketException e) {
//					logger.d("#sendUDPRequest#SocketException !111111111111");
//
//					e.printStackTrace();
//				} catch (IOException e) {
//					logger.d("#sendUDPRequest#IOException !111111111111");
//
//					e.printStackTrace();
//				}
//				}
//			}.start();


		return true;
	}
}
