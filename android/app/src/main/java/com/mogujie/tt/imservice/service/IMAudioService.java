package com.mogujie.tt.imservice.service;

import android.app.IntentService;
import android.content.Intent;

import com.mogujie.tt.utils.Logger;

/**
 * 实时语音服务
 * @author : xieqq on 2016-06-17.
 * @email : 342418923@qq.com
 * 异步的startService请求，IntentService会处理完成一个之后再处理第二个
 * 每一个请求都会在一个单独的worker thread中处理，不会阻塞应用程序的主线程
 * 耗时的操作与其在Service里面开启新线程还不如使用IntentService来处理耗时操作。
 * 总结IntentService的特征有：
（1）会创建独立的worker线程来处理所有的Intent请求；
（2）会创建独立的worker线程来处理onHandleIntent()方法实现的代码，无需处理多线程问题；
（3）所有请求处理完成后，IntentService会自动停止，无需调用stopSelf()方法停止Service；
 */
public class IMAudioService extends IntentService {

    private static Logger logger = Logger.getLogger(IMAudioService.class);

    public IMAudioService(){
        super("IMAudioService");
    }

    public IMAudioService(String name) {
        super(name);
    }

    /**
     * This method is invoked on the worker thread with a request to process.
     * Only one Intent is processed at a time, but the processing happens on a
     * worker thread that runs independently from other application logic.
     * So, if this code takes a long time, it will hold up other requests to
     * the same IntentService, but it will not hold up anything else.
     * When all requests have been handled, the IntentService stops itself,
     * so you should not call {@link #stopSelf}.
     *
     * @param intent The value passed to {@link
     *               android.content.Context#startService(Intent)}.
     */
    @Override
    protected void onHandleIntent(Intent intent) {
        // 发送线程
        while(true){

        }
    }
}
