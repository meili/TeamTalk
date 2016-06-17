
package com.mogujie.tt.ui.helper;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

import com.mogujie.tt.imservice.support.audio.Speex;
import com.mogujie.tt.utils.Logger;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

/**
 * 播放的线程
 * 网络接收后put数据
 */
public class AudioNetPlayHandler implements Runnable {

    private Logger logger = Logger.getLogger(AudioNetPlayHandler.class);
    private volatile boolean isPlaying;
    private final Object mutex = new Object();
    private static final int frequency = 8000;
    private static final int audioEncoding = AudioFormat.ENCODING_PCM_16BIT;
//    public static int packagesize = 160;// 320
//    private String fileName = null;
//    private float recordTime = 0;
//    private long startTime = 0;
//    private long endTime = 0;
//    private long maxVolumeStart = 0;
//    private long maxVolumeEnd = 0;
//    private static AudioRecord recordInstance = null;
//    private boolean bIMAudio = false;

    private AudioTrack track = null;
    public Speex speexDecoder;

    List<ReadAData> list = null;

    public AudioNetPlayHandler() {
        super();
//        SpeexDecoder speexdec = new SpeexDecoder(null);

        if(track == null) {
            int sampleRate = 8000;
            int minBufferSize = AudioTrack.getMinBufferSize(sampleRate,
                    AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT);
            // 实时播放
            track = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
                    AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT,
                    minBufferSize, AudioTrack.MODE_STREAM);
        }
        // 解码的
        speexDecoder = new Speex();
        speexDecoder.init();

        list = Collections.synchronizedList(new LinkedList<ReadAData>());
    }

    public void putData(byte[] payload) {
        int decsize = 0;
        short[] decoded = new short[160];
        if ((decsize = speexDecoder.decode(payload, decoded,160)) > 0) {
            ReadAData rd = new ReadAData(decsize);
            synchronized (mutex) {
                rd.size = decsize;
                System.arraycopy(decoded, 0, rd.ready, 0, decsize);
                list.add(rd);
            }
        }
    }

    class ReadAData {
        private int size;
        private byte[] ready;// = new byte[encoder_packagesize];
        ReadAData(int nsize){
            size = nsize;
            ready = new byte[nsize];
        }
    }

    public void run() {
        try {
            while (isPlaying) {
                if (list.size() == 0) {
                    logger.d("no data need to do play");
                    try {
                        Thread.sleep(20);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    continue;
                }

                if (list.size() > 0) {
                    synchronized (mutex) {
                        ReadAData rawdata = list.remove(0);
                        track.write(rawdata.ready,0,rawdata.size);
                        track.play(); // 播放
                    }
                }
            }
        } catch (Exception e) {
            logger.e(e.getMessage());
        } finally {
            if (track != null) {
                track.stop();
                track.release();
            }
        }

    }

    public void setPlaying(boolean isRec) {
        synchronized (mutex) {
            this.isPlaying = isRec;
            if (this.isPlaying) {
                mutex.notify();
            }
        }
    }

    public boolean isPlaying() {
        synchronized (mutex) {
            return isPlaying;
        }
    }
}
