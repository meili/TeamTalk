
package com.mogujie.tt.imservice.support.audio;

import com.mogujie.tt.imservice.manager.IMNatServerManager;
import com.mogujie.tt.utils.Logger;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

public class SpeexNetWriter implements Runnable {

    private Logger log = Logger.getLogger(SpeexNetWriter.class);
    private final Object mutex = new Object();

    private volatile boolean isRecording;
    private processedData pData;
    private List<processedData> list;


    public static int write_packageSize = 1024;

    public SpeexNetWriter() {
        super();
        list = Collections.synchronizedList(new LinkedList<processedData>());
    }

    public void run() {
        log.d("udp send thread runing");
        while (this.isRecording() || list.size() > 0) {

            if (list.size() > 0) {
                pData = list.remove(0);
                // gauss_packageSize/2
                log.i("pData size=" + pData.size + "_" + pData.processed.length);
                IMNatServerManager.instance().SendRealAudioData(pData.processed,pData.size);
                log.d("list size = {}" + list.size());
            } else {
                try {
                    Thread.sleep(20);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

        }

        log.d("write thread exit");
//        stop();
    }

    public void putData(final byte[] buf, int size) {

        log.d("after convert. size=====================[640]:" + size);

        processedData data = new processedData();
        // data.ts = ts;
        data.size = size;
        // 第一个是要复制的数组，第二个是从要复制的数组的第几个开始，第三个是复制到那，
        // 四个是复制到的数组第几个开始，最后一个是复制长度
        System.arraycopy(buf, 0, data.processed, 0, size);
        list.add(data);
    }
//
//    public void stop() {
//    }

    public void setRecording(boolean isRecording) {
        synchronized (mutex) {
            this.isRecording = isRecording;
            if (this.isRecording) {
                mutex.notify();
            }
        }
    }

    public boolean isRecording() {
        synchronized (mutex) {
            return isRecording;
        }
    }

    class processedData {
        // private long ts;
        private int size;
        private byte[] processed = new byte[write_packageSize];
    }

}
