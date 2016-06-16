
package com.mogujie.tt.imservice.support.audio;

import com.mogujie.tt.utils.Logger;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

public class SpeexEncoder implements Runnable {

    private Logger log = Logger.getLogger(SpeexEncoder.class);
    private final Object mutex = new Object();
    private Speex speex = new Speex();

    public static int encoder_packagesize = 1024;
    private byte[] processedData = new byte[encoder_packagesize];

    List<ReadData> list = null;
    private volatile boolean isRecording;
    private String fileName;

    public SpeexEncoder(String fileName) {
        super();
        speex.init();
        list = Collections.synchronizedList(new LinkedList<ReadData>());
        this.fileName = fileName;
    }

    public void run() {
        SpeexWriter fileWriter = new SpeexWriter(fileName);
        Thread consumerThread = new Thread((Runnable) fileWriter);
        fileWriter.setRecording(true);
        consumerThread.start();

        android.os.Process
                .setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);

        int getSize = 0;
        while (this.isRecording()) {
            if (list.size() == 0) {
                log.d("no data need to do encode");
                try {
                    Thread.sleep(20);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                continue;
            }
            if (list.size() > 0) {
                synchronized (mutex) {
                    ReadData rawdata = list.remove(0);
                    short[] playData = new short[rawdata.size];
                    getSize = speex.encode(rawdata.ready, 0, processedData,
                            rawdata.size);
                    log.i("after encode......................before="
                            + rawdata.size + " after=" + processedData.length
                            + " getsize=" + getSize);
                }
                if (getSize > 0) {

                    // 如果是实时语音  // 发送 processdData

                    fileWriter.putData(processedData, getSize);
                    log.i("............onLoginOut....................");
                    processedData = new byte[encoder_packagesize];
                }
            }
        }
        log.d("encode thread exit");
        fileWriter.setRecording(false);
        speex.close();
    }

    public void putData(short[] data, int size) {
        ReadData rd = new ReadData();
        synchronized (mutex) {
            rd.size = size;
            System.arraycopy(data, 0, rd.ready, 0, size);
            list.add(rd);
        }
    }

    public void setRecording(boolean isRecording) {
        synchronized (mutex) {
            this.isRecording = isRecording;
        }
    }

    public boolean isRecording() {
        synchronized (mutex) {
            return isRecording;
        }
    }

    class ReadData {
        private int size;
        private short[] ready = new short[encoder_packagesize];
    }
}
