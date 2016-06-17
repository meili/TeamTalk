package com.mogujie.tt.imservice.manager;

import com.mogujie.tt.ui.helper.AudioRecordHandler;
import com.mogujie.tt.utils.Logger;


public class IMAudioManager extends  IMManager{
    private Logger logger = Logger.getLogger(IMContactManager.class);

    private AudioRecordHandler audioRecorderInstance = null;
    private Thread audioRecorderThread = null;

    private static IMAudioManager inst = new IMAudioManager();
    public static IMAudioManager instance() {
        return inst;
    }

    @Override
    public void doOnStart() {
    }

    @Override
    public void reset() {
    }

    /**
     * 开始发送
     */
    public void startIMAudioSend() {
        if(audioRecorderInstance == null) {
            audioRecorderInstance = new AudioRecordHandler(null, true);
            audioRecorderThread = new Thread(audioRecorderInstance);
            audioRecorderInstance.setRecording(true); // 开始采集音频
            logger.d("IMAudioManager#audio#audio im thread starts");
            audioRecorderThread.start();
        }
    }

    /**
     * 停止发送
     */
    public void stopIMAudio(){
        if(audioRecorderInstance != null){
            if (audioRecorderInstance.isRecording()) {
                audioRecorderInstance.setRecording(false);
            }
            audioRecorderInstance=null;
        }
    }
}
