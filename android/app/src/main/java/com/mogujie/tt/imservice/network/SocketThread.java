package com.mogujie.tt.imservice.network;

import com.google.protobuf.GeneratedMessageLite;
import com.mogujie.tt.config.SysConstant;
import com.mogujie.tt.protobuf.base.DataBuffer;
import com.mogujie.tt.protobuf.base.Header;
import com.mogujie.tt.utils.Logger;

import org.jboss.netty.bootstrap.ClientBootstrap;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelFuture;
import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.channel.SimpleChannelHandler;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;
import org.jboss.netty.handler.codec.frame.LengthFieldBasedFrameDecoder;

import java.net.InetSocketAddress;
import java.util.concurrent.Executors;

public class SocketThread extends Thread {
	private ClientBootstrap clientBootstrap = null;
	private ChannelFactory channelFactory = null;
	private ChannelFuture channelFuture = null;
	private Channel channel = null;
	private String strHost = null;
	private int nPort = 0;
	private static Logger logger = Logger.getLogger(SocketThread.class);

	public SocketThread(String strHost, int nPort, SimpleChannelHandler handler) {
		this.strHost = strHost;
		this.nPort = nPort;
		init(handler);
	}

	@Override
	public void run() {
		doConnect();
	}

	private void init(final SimpleChannelHandler handler) {
		// only one IO thread
		channelFactory = new NioClientSocketChannelFactory(
		Executors.newSingleThreadExecutor(),
		Executors.newSingleThreadExecutor());

		clientBootstrap = new ClientBootstrap(channelFactory); // 用的netty的库
		clientBootstrap.setOption("connectTimeoutMillis", 5000);
		clientBootstrap.setPipelineFactory(new ChannelPipelineFactory() {

			public ChannelPipeline getPipeline() throws Exception {
				ChannelPipeline pipeline = Channels.pipeline();
				// 接收的数据包解码
				pipeline.addLast("decoder", new LengthFieldBasedFrameDecoder(
				400 * 1024, 0, 4, -4, 0));
				// 第一个参数为信息最大长度  第二参数为长度属性的起始
				// 第三个参数为“长度属性”的长度  第四个参数为长度调节值  第五个参数为跳过的字节数

				// 发送的数据包编码
				//pipeline.addLast("encoder", new PacketEncoder());
				// 具体的业务处理，这个handler只负责接收数据，并传递给dispatcher
				pipeline.addLast("handler", handler);
				return pipeline;

			}

		});

		clientBootstrap.setOption("tcpNoDelay", true);
		clientBootstrap.setOption("keepAlive", true);
		// clientBootstrap.setOption("keepIdle", 20);
		// clientBootstrap.setOption("keepInterval", 5);
		// clientBootstrap.setOption("keepCount", 3);
	}

	public boolean doConnect() {
		try {
			if ((null == channel || (null != channel && !channel.isConnected()))
					&& null != this.strHost && this.nPort > 0) {
				// Start the connection attempt.
				channelFuture = clientBootstrap.connect(new InetSocketAddress(
				strHost, nPort));
				// Wait until the connection attempt succeeds or fails.
				channel = channelFuture.awaitUninterruptibly().getChannel();
				if (!channelFuture.isSuccess()) {
					channelFuture.getCause().printStackTrace();
					clientBootstrap.releaseExternalResources();
					// ReconnectManager.getInstance().setOnRecconnecting(false);
					// ReconnectManager.getInstance().setLogining(false);
					return false;
			    }
			}

			// Wait until the connection is closed or the connection attemp
			// fails.
			channelFuture.getChannel().getCloseFuture().awaitUninterruptibly();
			// Shut down thread pools to exit.
			clientBootstrap.releaseExternalResources();
			return true;

		} catch (Exception e) {
			logger.e("do connect failed. e: %s", e.getStackTrace().toString());
			return false;
		}
	}

	public Channel getChannel() {
		return channel;
	}

	public void close() {
		if (null == channelFuture)
			return;
		if (null != channelFuture.getChannel()) {
			channelFuture.getChannel().close();
		}
        channelFuture.cancel();
	}


    // todo check
    @Deprecated
    public boolean isClose(){
        if(channelFuture != null && channelFuture.getChannel() != null){
            return !channelFuture.getChannel().isConnected();
        }
        return true;
    }

    /**
     * @param requset
     * @param header
     * @return
     */
    public boolean send_request(GeneratedMessageLite requset,Header header){
        DataBuffer headerBuffer = header.encode();
        DataBuffer bodyBuffer = new DataBuffer();
        int bodySize = requset.getSerializedSize();
        bodyBuffer.writeBytes(requset.toByteArray());


        DataBuffer buffer = new DataBuffer(SysConstant.PROTOCOL_HEADER_LENGTH  + bodySize);
        buffer.writeDataBuffer(headerBuffer);
        buffer.writeDataBuffer(bodyBuffer);

        if (null != buffer && null != channelFuture.getChannel()) {
            /**底层的状态要提前判断，netty抛出的异常上层catch不到*/
            Channel currentChannel =  channelFuture.getChannel();
            boolean isW = currentChannel.isWritable();
            boolean isC  = currentChannel.isConnected();
            if(!(isW && isC)){
                throw  new RuntimeException("#sendRequest#channel is close!");
            }
            channelFuture.getChannel().write(buffer.getOrignalBuffer());
            logger.d("packet#send ok");
            return true;
        } else {
            logger.e("packet#send failed");
            return false;
        }
    }

}
