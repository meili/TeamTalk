package com.mogujie.tt.imservice.callback;

import android.os.Handler;

import com.mogujie.tt.utils.Logger;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * @author : yingmu on 15-1-7.
 * @email : yingmu@mogujie.com.
 * postDelayed 五秒执行一次，会把没有反馈的超时pop
 */
public class ListenerQueue {

    private static ListenerQueue listenerQueue = new ListenerQueue();
    private Logger logger = Logger.getLogger(ListenerQueue.class);
    public static ListenerQueue instance(){
        return listenerQueue;
    }

    private volatile  boolean stopFlag = false;
    private volatile  boolean hasTask = false;


    //callback 队列
    private Map<Integer,Packetlistener> callBackQueue = new ConcurrentHashMap<>();
    private Handler timerHandler = new Handler();

    public void onStart(){
        logger.d("ListenerQueue#onStart run");
        stopFlag = false;
        startTimer();
    }
    public void onDestory(){
        logger.d("ListenerQueue#onDestory ");
        callBackQueue.clear();
        stopTimer();
    }

    //以前是TimerTask处理方式
    private void startTimer() {
        if(!stopFlag && hasTask == false) {
            hasTask = true;
            timerHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    timerImpl();
                    hasTask = false;
                    startTimer();
                }
            }, 5 * 1000);
        }
    }

    private void stopTimer(){
        stopFlag = true;
    }

    private void timerImpl() {
        long currentRealtime =   System.currentTimeMillis();//SystemClock.elapsedRealtime();

        for (java.util.Map.Entry<Integer, Packetlistener> entry : callBackQueue.entrySet()) {

            Packetlistener packetlistener = entry.getValue();
            Integer seqNo = entry.getKey();
            long timeRange = currentRealtime - packetlistener.getCreateTime();

            try {
                if (timeRange >= packetlistener.getTimeOut()) {
                    logger.d("ListenerQueue#find timeout msg");
                    Packetlistener listener = pop(seqNo);
                    if (listener != null) {
                        listener.onTimeout();
                    }
                }
            } catch (Exception e) {
                logger.d("ListenerQueue#timerImpl onTimeout is Error,exception is %s", e.getCause());
            }
        }
    }

    public void push(int seqNo,Packetlistener packetlistener){
        if(seqNo <=0 || null==packetlistener){
            logger.d("ListenerQueue#push error, cause by Illegal params");
            return;
        }
        callBackQueue.put(seqNo,packetlistener);
    }


    public Packetlistener pop(int seqNo){
        synchronized (ListenerQueue.this) {
            if (callBackQueue.containsKey(seqNo)) {
                Packetlistener packetlistener = callBackQueue.remove(seqNo);
                return packetlistener;
            }
            return null;
        }
    }
}
