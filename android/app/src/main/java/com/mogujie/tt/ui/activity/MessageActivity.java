
package com.mogujie.tt.ui.activity;

import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore;
import android.provider.Settings;
import android.text.Editable;
import android.text.Selection;
import android.text.TextWatcher;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.AbsListView;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RadioGroup;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;
import android.widget.TextView;
import android.widget.Toast;

import com.handmark.pulltorefresh.library.PullToRefreshBase;
import com.handmark.pulltorefresh.library.PullToRefreshBase.OnRefreshListener2;
import com.handmark.pulltorefresh.library.PullToRefreshListView;
import com.mogujie.tt.DB.entity.GroupEntity;
import com.mogujie.tt.DB.entity.MessageEntity;
import com.mogujie.tt.DB.entity.PeerEntity;
import com.mogujie.tt.DB.entity.UserEntity;
import com.mogujie.tt.DB.sp.SystemConfigSp;
import com.mogujie.tt.R;
import com.mogujie.tt.app.IMApplication;
import com.mogujie.tt.config.DBConstant;
import com.mogujie.tt.config.HandlerConstant;
import com.mogujie.tt.config.IntentConstant;
import com.mogujie.tt.config.SysConstant;
import com.mogujie.tt.imservice.entity.AudioMessage;
import com.mogujie.tt.imservice.entity.ImageMessage;
import com.mogujie.tt.imservice.entity.TextMessage;
import com.mogujie.tt.imservice.entity.UnreadEntity;
import com.mogujie.tt.imservice.event.MessageEvent;
import com.mogujie.tt.imservice.event.PriorityEvent;
import com.mogujie.tt.imservice.event.SelectEvent;
import com.mogujie.tt.imservice.manager.IMLoginManager;
import com.mogujie.tt.imservice.manager.IMStackManager;
import com.mogujie.tt.imservice.service.IMService;
import com.mogujie.tt.imservice.support.IMServiceConnector;
import com.mogujie.tt.ui.adapter.MessageAdapter;
import com.mogujie.tt.ui.adapter.album.AlbumHelper;
import com.mogujie.tt.ui.adapter.album.ImageBucket;
import com.mogujie.tt.ui.adapter.album.ImageItem;
import com.mogujie.tt.ui.base.TTBaseActivity;
import com.mogujie.tt.ui.helper.AudioPlayerHandler;
import com.mogujie.tt.ui.helper.AudioRecordHandler;
import com.mogujie.tt.ui.helper.Emoparser;
import com.mogujie.tt.ui.widget.CustomEditView;
import com.mogujie.tt.ui.widget.EmoGridView;
import com.mogujie.tt.ui.widget.EmoGridView.OnEmoGridViewItemClick;
import com.mogujie.tt.ui.widget.MGProgressbar;
import com.mogujie.tt.ui.widget.YayaEmoGridView;
import com.mogujie.tt.utils.CommonUtil;
import com.mogujie.tt.utils.IMUIHelper;
import com.mogujie.tt.utils.Logger;
import com.nostra13.universalimageloader.core.ImageLoader;
import com.nostra13.universalimageloader.core.listener.PauseOnScrollListener;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;

import de.greenrobot.event.EventBus;

/**
 * @author Nana
 * @Description 主消息界面
 * @date 2014-7-15
 * <p/>
 */
public class MessageActivity extends TTBaseActivity
        implements
        OnRefreshListener2<ListView>,
        View.OnClickListener,
        OnTouchListener,
        TextWatcher,
        SensorEventListener {

    private static Handler uiHandler = null;// 处理语音


    private PullToRefreshListView lvPTR = null;
    private CustomEditView messageEdt = null;
    private TextView sendBtn = null;
    private Button recordAudioBtn = null;
    private ImageView keyboardInputImg = null;
    private ImageView soundVolumeImg = null;
    private LinearLayout soundVolumeLayout = null;


    private ImageView audioInputImg = null;
    private ImageView addPhotoBtn = null;
    private ImageView addEmoBtn = null;
    private LinearLayout emoLayout = null;
    private EmoGridView emoGridView = null;
    private YayaEmoGridView yayaEmoGridView = null;
    private RadioGroup emoRadioGroup = null;
    private String audioSavePath = null;
    private InputMethodManager inputManager = null;
    private AudioRecordHandler audioRecorderInstance = null;
    private TextView textView_new_msg_tip = null;

    private MessageAdapter adapter = null;
    private Thread audioRecorderThread = null;
    private Dialog soundVolumeDialog = null;
    private View addOthersPanelView = null;
    private AlbumHelper albumHelper = null;


    private List<ImageBucket> albumList = null;
    MGProgressbar progressbar = null;

    //private boolean audioReday = false; 语音先关的
    private SensorManager sensorManager = null;
    private Sensor sensor = null;


    private String takePhotoSavePath = "";
    private Logger logger = Logger.getLogger(MessageActivity.class);
    private IMService imService;
    private UserEntity loginUser;
    private PeerEntity peerEntity;

    // 当前的session
    private String currentSessionKey;
    private int historyTimes = 0;

    //键盘布局相关参数
    int rootBottom = Integer.MIN_VALUE, keyboardHeight = 0;
    switchInputMethodReceiver receiver;
    private String currentInputMethod;

    /**
     * 全局Toast
     */
    private Toast mToast;

    public void showToast(int resId) {
        String text = getResources().getString(resId);
        if (mToast == null) {
            mToast = Toast.makeText(MessageActivity.this, text, Toast.LENGTH_SHORT);
        } else {
            mToast.setText(text);
            mToast.setDuration(Toast.LENGTH_SHORT);
        }
        mToast.setGravity(Gravity.CENTER, 0, 0);
        mToast.show();
    }

    public void cancelToast() {
        if (mToast != null) {
            mToast.cancel();
        }
    }

    @Override
    public void onBackPressed() {
        IMApplication.gifRunning = false;
        cancelToast();
        super.onBackPressed();
    }

    /**
     * end 全局Toast
     */
    private IMServiceConnector imServiceConnector = new IMServiceConnector() {
        @Override
        public void onIMServiceConnected() {
            logger.d("message_activity#onIMServiceConnected");
            imService = imServiceConnector.getIMService();
            initData();
        }

        @Override
        public void onServiceDisconnected() {
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        logger.d("message_activity#onCreate:%s", this);
        super.onCreate(savedInstanceState);
        currentSessionKey = getIntent().getStringExtra(IntentConstant.KEY_SESSION_KEY);
        initSoftInputMethod();
        initEmo();
        initAlbumHelper();
        initAudioHandler();
        initAudioSensor();
        initView();
        imServiceConnector.connect(this);
        EventBus.getDefault().register(this, SysConstant.MESSAGE_EVENTBUS_PRIORITY);
        logger.d("message_activity#register im service and eventBus");
    }

    // 触发条件,imservice链接成功，或者newIntent
    private void initData() {
        historyTimes = 0;
        adapter.clearItem();
        ImageMessage.clearImageMessageList();
        loginUser = imService.getLoginManager().getLoginInfo();
        peerEntity = imService.getSessionManager().findPeerEntity(currentSessionKey);
        // 头像、历史消息加载、取消通知
        setTitleByUser();
        reqHistoryMsg();
        adapter.setImService(imService, loginUser);
        imService.getUnReadMsgManager().readUnreadSession(currentSessionKey);
        imService.getNotificationManager().cancelSessionNotifications(currentSessionKey);
    }

    private void initSoftInputMethod() {
        this.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        inputManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);

        receiver = new switchInputMethodReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction("android.intent.action.INPUT_METHOD_CHANGED");
        registerReceiver(receiver, filter);

        SystemConfigSp.instance().init(this);
        currentInputMethod = Settings.Secure.getString(MessageActivity.this.getContentResolver(), Settings.Secure.DEFAULT_INPUT_METHOD);
        keyboardHeight = SystemConfigSp.instance().getIntConfig(currentInputMethod);
    }

    /**
     * 本身位于Message页面，点击通知栏其他session的消息
     */
    @Override
    protected void onNewIntent(Intent intent) {
        logger.d("message_activity#onNewIntent:%s", this);
        super.onNewIntent(intent);
        setIntent(intent);
        historyTimes = 0;
        if (intent == null) {
            return;
        }
        String newSessionKey = getIntent().getStringExtra(IntentConstant.KEY_SESSION_KEY);
        if (newSessionKey == null) {
            return;
        }
        logger.d("chat#newSessionInfo:%s", newSessionKey);
        if (!newSessionKey.equals(currentSessionKey)) {
            currentSessionKey = newSessionKey;
            initData();
        }
    }

    @Override
    protected void onResume() {
        logger.d("message_activity#onresume:%s", this);
        super.onResume();
        IMApplication.gifRunning = true;
        historyTimes = 0;
        // not the first time
        if (imService != null) {
            // 处理session的未读信息
            handleUnreadMsgs();
        }
    }

    @Override
    protected void onDestroy() {
        logger.d("message_activity#onDestroy:%s", this);
        historyTimes = 0;
        imServiceConnector.disconnect(this);
        EventBus.getDefault().unregister(this);
        adapter.clearItem();
        albumList.clear();
        sensorManager.unregisterListener(this, sensor);
        ImageMessage.clearImageMessageList();
        unregisterReceiver(receiver);
        super.onDestroy();
    }

    /**
     * 设定聊天名称
     * 1. 如果是user类型， 点击触发UserProfile
     * 2. 如果是群组，检测自己是不是还在群中
     */
    private void setTitleByUser() {
        setTitle(peerEntity.getMainName());
        int peerType = peerEntity.getType();
        switch (peerType) {
            case DBConstant.SESSION_TYPE_GROUP: {
                GroupEntity group = (GroupEntity) peerEntity;
                Set<Integer> memberLists = group.getlistGroupMemberIds();
                if (!memberLists.contains(loginUser.getPeerId())) {
                    Toast.makeText(MessageActivity.this, R.string.no_group_member, Toast.LENGTH_SHORT).show();
                }
            }
            break;
            case DBConstant.SESSION_TYPE_SINGLE: {
                topTitleTxt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        IMUIHelper.openUserProfileActivity(MessageActivity.this, peerEntity.getPeerId());
                    }
                });
            }
            break;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (RESULT_OK != resultCode)
            return;
        switch (requestCode) {
            case SysConstant.CAMERA_WITH_DATA:
                handleTakePhotoData(data);
                break;
            case SysConstant.ALBUM_BACK_DATA:
                logger.d("pic#ALBUM_BACK_DATA");
                setIntent(data);
                break;
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    private void handleImagePickData(List<ImageItem> list) {
        ArrayList<ImageMessage> listMsg = new ArrayList<>();
        ArrayList<ImageItem> itemList = (ArrayList<ImageItem>) list;
        for (ImageItem item : itemList) {
            ImageMessage imageMessage = ImageMessage.buildForSend(item, loginUser, peerEntity);
            listMsg.add(imageMessage);
            pushList(imageMessage);
        }
        imService.getMessageManager().sendImages(listMsg);
    }


    public void onEventMainThread(SelectEvent event) {
        List<ImageItem> itemList = event.getList();
        if (itemList != null || itemList.size() > 0)
            handleImagePickData(itemList);
    }

    /**
     * 背景: 1.EventBus的cancelEventDelivery的只能在postThread中运行，而且没有办法绕过这一点
     * 2. onEvent(A a)  onEventMainThread(A a) 这个两个是没有办法共存的
     * 解决: 抽离出那些需要优先级的event，在onEvent通过handler调用主线程，
     * 然后cancelEventDelivery
     * <p/>
     * todo  need find good solution
     */
    public void onEvent(PriorityEvent event) {
        switch (event.event) {
            case MSG_RECEIVED_MESSAGE: {
                MessageEntity entity = (MessageEntity) event.object;
                /**正式当前的会话*/
                if (currentSessionKey.equals(entity.getSessionKey())) {
                    Message message = Message.obtain();
                    message.what = HandlerConstant.MSG_RECEIVED_MESSAGE;
                    message.obj = entity;
                    uiHandler.sendMessage(message);
                    EventBus.getDefault().cancelEventDelivery(event);
                }
            }
            break;
        }
    }

    public void onEventMainThread(MessageEvent event) {
        MessageEvent.Event type = event.getEvent();
        MessageEntity entity = event.getMessageEntity();
        switch (type) {
            case ACK_SEND_MESSAGE_OK: {
                onMsgAck(event.getMessageEntity());
            }
            break;

            case ACK_SEND_MESSAGE_FAILURE:
                // 失败情况下新添提醒
                showToast(R.string.message_send_failed);
            case ACK_SEND_MESSAGE_TIME_OUT: {
                onMsgUnAckTimeoutOrFailure(event.getMessageEntity());
            }
            break;

            case HANDLER_IMAGE_UPLOAD_FAILD: {
                logger.d("pic#onUploadImageFaild");
                ImageMessage imageMessage = (ImageMessage) event.getMessageEntity();
                adapter.updateItemState(imageMessage);
                showToast(R.string.message_send_failed);
            }
            break;

            case HANDLER_IMAGE_UPLOAD_SUCCESS: {
                ImageMessage imageMessage = (ImageMessage) event.getMessageEntity();
                adapter.updateItemState(imageMessage);
            }
            break;

            case HISTORY_MSG_OBTAIN: {
                if (historyTimes == 1) {
                    adapter.clearItem();
                    reqHistoryMsg();
                }
            }
            break;
        }
    }

    /**
     * audio状态的语音还在使用这个
     */
    protected void initAudioHandler() {
        uiHandler = new Handler() {
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                switch (msg.what) {
                    case HandlerConstant.HANDLER_RECORD_FINISHED:
                        onRecordVoiceEnd((Float) msg.obj);
                        break;

                    // 录音结束
                    case HandlerConstant.HANDLER_STOP_PLAY:
                        // 其他地方处理了
                        //adapter.stopVoicePlayAnim((String) msg.obj);
                        break;

                    case HandlerConstant.RECEIVE_MAX_VOLUME:
                        onReceiveMaxVolume((Integer) msg.obj);
                        break;

                    case HandlerConstant.RECORD_AUDIO_TOO_LONG:
                        doFinishRecordAudio();
                        break;

                    case HandlerConstant.MSG_RECEIVED_MESSAGE:
                        MessageEntity entity = (MessageEntity) msg.obj;
                        onMsgRecv(entity);
                        break;

                    default:
                        break;
                }
            }
        };
    }

    /**
     * [备注] DB保存，与session的更新manager已经做了
     *
     * @param messageEntity
     */
    private void onMsgAck(MessageEntity messageEntity) {
        logger.d("message_activity#onMsgAck");
        int msgId = messageEntity.getMsgId();
        logger.d("chat#onMsgAck, msgId:%d", msgId);

        /**到底采用哪种ID呐??*/
        long localId = messageEntity.getId();
        adapter.updateItemState(messageEntity);
    }


    private void handleUnreadMsgs() {
        logger.d("messageacitivity#handleUnreadMsgs sessionId:%s", currentSessionKey);
        // 清除未读消息
        UnreadEntity unreadEntity = imService.getUnReadMsgManager().findUnread(currentSessionKey);
        if (null == unreadEntity) {
            return;
        }
        int unReadCnt = unreadEntity.getUnReadCnt();
        if (unReadCnt > 0) {
            imService.getNotificationManager().cancelSessionNotifications(currentSessionKey);
            adapter.notifyDataSetChanged();
            scrollToBottomListItem();
        }
    }


    // 肯定是在当前的session内
    private void onMsgRecv(MessageEntity entity) {
        logger.d("message_activity#onMsgRecv");

        imService.getUnReadMsgManager().ackReadMsg(entity);
        logger.d("chat#start pushList");
        pushList(entity);
        ListView lv = lvPTR.getRefreshableView();
        if (lv != null) {

            if (lv.getLastVisiblePosition() < adapter.getCount()) {
                textView_new_msg_tip.setVisibility(View.VISIBLE);
            } else {
                scrollToBottomListItem();
            }
        }
    }


    private void onMsgUnAckTimeoutOrFailure(MessageEntity messageEntity) {
        logger.d("chat#onMsgUnAckTimeoutOrFailure, msgId:%s", messageEntity.getMsgId());
        // msgId 应该还是为0
        adapter.updateItemState(messageEntity);
    }


    /**
     * @Description 显示联系人界面
     */
    private void showGroupManageActivity() {
        Intent i = new Intent(this, GroupManagermentActivity.class);
        i.putExtra(IntentConstant.KEY_SESSION_KEY, currentSessionKey);
        startActivity(i);
    }

    /**
     * @Description 初始化AudioManager，用于访问控制音量和钤声模式
     */
    private void initAudioSensor() {
        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
        sensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_NORMAL);
    }

    /**
     * @Description 初始化数据（相册,表情,数据库相关）
     */
    private void initAlbumHelper() {
        albumHelper = AlbumHelper.getHelper(MessageActivity.this);
        albumList = albumHelper.getImagesBucketList(false);
    }

    private void initEmo() {
        Emoparser.getInstance(MessageActivity.this);
        IMApplication.gifRunning = true;
    }

    /**
     * @Description 初始化界面控件
     * 有点庞大 todo
     */
    private void initView() {
        // 绑定布局资源(注意放所有资源初始化之前)
        LayoutInflater.from(this).inflate(R.layout.tt_activity_message, topContentView);

        //TOP_CONTENT_VIEW
        setLeftButton(R.drawable.tt_top_back);
        setLeftText(getResources().getString(R.string.top_left_back));
        setRightButton(R.drawable.tt_top_right_group_manager);
        topLeftBtn.setOnClickListener(this);
        letTitleTxt.setOnClickListener(this);
        topRightBtn.setOnClickListener(this);

        // 列表控件(开源PTR)
        lvPTR = (PullToRefreshListView) this.findViewById(R.id.message_list);
        textView_new_msg_tip = (TextView) findViewById(R.id.tt_new_msg_tip);
        lvPTR.getRefreshableView().addHeaderView(LayoutInflater.from(this).inflate(R.layout.tt_messagelist_header,lvPTR.getRefreshableView(), false));
        Drawable loadingDrawable = getResources().getDrawable(R.drawable.pull_to_refresh_indicator);
        final int indicatorWidth = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 29,
                getResources().getDisplayMetrics());
        loadingDrawable.setBounds(new Rect(0, indicatorWidth, 0, indicatorWidth));
        lvPTR.getLoadingLayoutProxy().setLoadingDrawable(loadingDrawable);
        lvPTR.getRefreshableView().setCacheColorHint(Color.WHITE);
        lvPTR.getRefreshableView().setSelector(new ColorDrawable(Color.WHITE));
        lvPTR.getRefreshableView().setOnTouchListener(lvPTROnTouchListener);
        adapter = new MessageAdapter(this);
        lvPTR.setAdapter(adapter);
        lvPTR.setOnRefreshListener(this);
        lvPTR.setOnScrollListener(new PauseOnScrollListener(ImageLoader.getInstance(), true, true) {
            @Override
            public void onScrollStateChanged(AbsListView view, int scrollState) {
                switch (scrollState) {
                    case AbsListView.OnScrollListener.SCROLL_STATE_IDLE:
                        if (view.getLastVisiblePosition() == (view.getCount() - 1)) {
                            textView_new_msg_tip.setVisibility(View.GONE);
                        }
                        break;
                }
            }
        });
        textView_new_msg_tip.setOnClickListener(this);

        // 界面底部输入框布局
        sendBtn = (TextView) this.findViewById(R.id.send_message_btn);
        recordAudioBtn = (Button) this.findViewById(R.id.record_voice_btn);
        audioInputImg = (ImageView) this.findViewById(R.id.voice_btn);
        messageEdt = (CustomEditView) this.findViewById(R.id.message_text);
        RelativeLayout.LayoutParams messageEdtParam = (LayoutParams) messageEdt.getLayoutParams();
        messageEdtParam.addRule(RelativeLayout.LEFT_OF, R.id.show_emo_btn);
        messageEdtParam.addRule(RelativeLayout.RIGHT_OF, R.id.voice_btn);
        keyboardInputImg = (ImageView) this.findViewById(R.id.show_keyboard_btn);
        addPhotoBtn = (ImageView) this.findViewById(R.id.show_add_photo_btn);
        addEmoBtn = (ImageView) this.findViewById(R.id.show_emo_btn);
        messageEdt.setOnFocusChangeListener(msgEditOnFocusChangeListener);
        messageEdt.setOnClickListener(this);
        messageEdt.addTextChangedListener(this);
        addPhotoBtn.setOnClickListener(this);
        addEmoBtn.setOnClickListener(this);
        keyboardInputImg.setOnClickListener(this);
        audioInputImg.setOnClickListener(this);
        recordAudioBtn.setOnTouchListener(this);
        sendBtn.setOnClickListener(this);
        initSoundVolumeDlg();

        //OTHER_PANEL_VIEW
        addOthersPanelView = findViewById(R.id.add_others_panel);
        LayoutParams params = (LayoutParams) addOthersPanelView.getLayoutParams();
        if (keyboardHeight > 0) {
            params.height = keyboardHeight;
            addOthersPanelView.setLayoutParams(params);
        }
        View takePhotoBtn = findViewById(R.id.take_photo_btn);
        View takeCameraBtn = findViewById(R.id.take_camera_btn);
        takePhotoBtn.setOnClickListener(this);
        takeCameraBtn.setOnClickListener(this);

        //EMO_LAYOUT
        emoLayout = (LinearLayout) findViewById(R.id.emo_layout);
        LayoutParams paramEmoLayout = (LayoutParams) emoLayout.getLayoutParams();
        if (keyboardHeight > 0) {
            paramEmoLayout.height = keyboardHeight;
            emoLayout.setLayoutParams(paramEmoLayout);
        }
        emoGridView = (EmoGridView) findViewById(R.id.emo_gridview);
        yayaEmoGridView = (YayaEmoGridView) findViewById(R.id.yaya_emo_gridview);
        emoRadioGroup = (RadioGroup) findViewById(R.id.emo_tab_group);
        emoGridView.setOnEmoGridViewItemClick(onEmoGridViewItemClick);
        emoGridView.setAdapter();
        yayaEmoGridView.setOnEmoGridViewItemClick(yayaOnEmoGridViewItemClick);
        yayaEmoGridView.setAdapter();
        emoRadioGroup.setOnCheckedChangeListener(emoOnCheckedChangeListener);


        //LOADING
        View view = LayoutInflater.from(MessageActivity.this)
                .inflate(R.layout.tt_progress_ly, null);
        progressbar = (MGProgressbar) view.findViewById(R.id.tt_progress);
        LayoutParams pgParms = new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT);
        pgParms.bottomMargin = 50;
        addContentView(view, pgParms);

        //ROOT_LAYOUT_LISTENER
        baseRoot.getViewTreeObserver().addOnGlobalLayoutListener(onGlobalLayoutListener);
    }

    /**
     * @Description 初始化音量对话框
     */
    private void initSoundVolumeDlg() {
        soundVolumeDialog = new Dialog(this, R.style.SoundVolumeStyle);
        soundVolumeDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        soundVolumeDialog.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        soundVolumeDialog.setContentView(R.layout.tt_sound_volume_dialog);
        soundVolumeDialog.setCanceledOnTouchOutside(true);
        soundVolumeImg = (ImageView) soundVolumeDialog.findViewById(R.id.sound_volume_img);
        soundVolumeLayout = (LinearLayout) soundVolumeDialog.findViewById(R.id.sound_volume_bk);
    }

    /**
     * 1.初始化请求历史消息
     * 2.本地消息不全，也会触发
     */
    private void reqHistoryMsg() {
        historyTimes++;
        List<MessageEntity> msgList = imService.getMessageManager().loadHistoryMsg(historyTimes,currentSessionKey,peerEntity);
        pushList(msgList);
        scrollToBottomListItem();
    }
    /**
     * @param msg
     */
    public void pushList(MessageEntity msg) {
        logger.d("chat#pushList msgInfo:%s", msg);
        adapter.addItem(msg);
    }

    public void pushList(List<MessageEntity> entityList) {
        logger.d("chat#pushList list:%d", entityList.size());
        adapter.loadHistoryList(entityList);
    }


    /**
     * @Description 录音超时(60s)，发消息调用该方法
     */
    public void doFinishRecordAudio() {
        try {
            if (audioRecorderInstance.isRecording()) {
                audioRecorderInstance.setRecording(false);
            }
            if (soundVolumeDialog.isShowing()) {
                soundVolumeDialog.dismiss();
            }

            recordAudioBtn.setBackgroundResource(R.drawable.tt_pannel_btn_voiceforward_normal);

            audioRecorderInstance.setRecordTime(SysConstant.MAX_SOUND_RECORD_TIME);
            onRecordVoiceEnd(SysConstant.MAX_SOUND_RECORD_TIME);
        } catch (Exception e) {
        }
    }

    /**
     * @param voiceValue
     * @Description 根据分贝值设置录音时的音量动画
     */
    private void onReceiveMaxVolume(int voiceValue) {
        if (voiceValue < 200.0) {
            soundVolumeImg.setImageResource(R.drawable.tt_sound_volume_01);
        } else if (voiceValue > 200.0 && voiceValue < 600) {
            soundVolumeImg.setImageResource(R.drawable.tt_sound_volume_02);
        } else if (voiceValue > 600.0 && voiceValue < 1200) {
            soundVolumeImg.setImageResource(R.drawable.tt_sound_volume_03);
        } else if (voiceValue > 1200.0 && voiceValue < 2400) {
            soundVolumeImg.setImageResource(R.drawable.tt_sound_volume_04);
        } else if (voiceValue > 2400.0 && voiceValue < 10000) {
            soundVolumeImg.setImageResource(R.drawable.tt_sound_volume_05);
        } else if (voiceValue > 10000.0 && voiceValue < 28000.0) {
            soundVolumeImg.setImageResource(R.drawable.tt_sound_volume_06);
        } else if (voiceValue > 28000.0) {
            soundVolumeImg.setImageResource(R.drawable.tt_sound_volume_07);
        }
    }


    @Override
    public void onConfigurationChanged(Configuration config) {
        super.onConfigurationChanged(config);
    }

    /**
     * @param data
     * @Description 处理拍照后的数据
     * 应该是从某个 activity回来的
     */
    private void handleTakePhotoData(Intent data) {
        ImageMessage imageMessage = ImageMessage.buildForSend(takePhotoSavePath, loginUser, peerEntity);
        List<ImageMessage> sendList = new ArrayList<>(1);
        sendList.add(imageMessage);
        imService.getMessageManager().sendImages(sendList);
        // 格式有些问题
        pushList(imageMessage);
        messageEdt.clearFocus();//消除焦点
    }

    /**
     * @param audioLen
     * @Description 录音结束后处理录音数据
     */
    private void onRecordVoiceEnd(float audioLen) {
        logger.d("message_activity#chat#audio#onRecordVoiceEnd audioLen:%f", audioLen);
        AudioMessage audioMessage = AudioMessage.buildForSend(audioLen, audioSavePath, loginUser, peerEntity);
        imService.getMessageManager().sendVoice(audioMessage);
        pushList(audioMessage);
    }

    @Override
    public void onPullUpToRefresh(PullToRefreshBase<ListView> refreshView) {
    }

    @Override
    public void onPullDownToRefresh(
            final PullToRefreshBase<ListView> refreshView) {
        // 获取消息
        refreshView.postDelayed(new Runnable() {
            @Override
            public void run() {
                ListView mlist = lvPTR.getRefreshableView();
                int preSum = mlist.getCount();
                MessageEntity messageEntity = adapter.getTopMsgEntity();
                if (messageEntity != null) {
                    List<MessageEntity> historyMsgInfo = imService.getMessageManager().loadHistoryMsg(messageEntity, historyTimes);
                    if (historyMsgInfo.size() > 0) {
                        historyTimes++;
                        adapter.loadHistoryList(historyMsgInfo);
                    }
                }

                int afterSum = mlist.getCount();
                mlist.setSelection(afterSum - preSum);
                /**展示位置为这次消息的最末尾*/
                //mlist.setSelection(size);
                // 展示顶部
//                if (!(mlist).isStackFromBottom()) {
//                    mlist.setStackFromBottom(true);
//                }
//                mlist.setStackFromBottom(false);
                refreshView.onRefreshComplete();
            }
        }, 200);
    }


    @Override
    public void onClick(View v) {
        final int id = v.getId();
        switch (id) {
            case R.id.left_btn:
            case R.id.left_txt:
                actFinish();
                break;
            case R.id.right_btn:
                showGroupManageActivity();
                break;
            case R.id.show_add_photo_btn: {
                recordAudioBtn.setVisibility(View.GONE);
                keyboardInputImg.setVisibility(View.GONE);
                messageEdt.setVisibility(View.VISIBLE);
                audioInputImg.setVisibility(View.VISIBLE);
                addEmoBtn.setVisibility(View.VISIBLE);

                if (keyboardHeight != 0) {
                    this.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING);
                }
                if (addOthersPanelView.getVisibility() == View.VISIBLE) {
                    if (!messageEdt.hasFocus()) {
                        messageEdt.requestFocus();
                    }
                    inputManager.toggleSoftInputFromWindow(messageEdt.getWindowToken(), 1, 0);
                    if (keyboardHeight == 0) {
                        addOthersPanelView.setVisibility(View.GONE);
                    }
                } else if (addOthersPanelView.getVisibility() == View.GONE) {
                    addOthersPanelView.setVisibility(View.VISIBLE);
                    inputManager.hideSoftInputFromWindow(messageEdt.getWindowToken(), 0);
                }
                if (null != emoLayout
                        && emoLayout.getVisibility() == View.VISIBLE) {
                    emoLayout.setVisibility(View.GONE);
                }

                scrollToBottomListItem();
            }
            break;
            case R.id.take_photo_btn: {
                if (albumList.size() < 1) {
                    Toast.makeText(MessageActivity.this,
                            getResources().getString(R.string.not_found_album), Toast.LENGTH_LONG)
                            .show();
                    return;
                }
                // 选择图片的时候要将session的整个回话 传过来
                Intent intent = new Intent(MessageActivity.this, PickPhotoActivity.class);
                intent.putExtra(IntentConstant.KEY_SESSION_KEY, currentSessionKey);
                startActivityForResult(intent, SysConstant.ALBUM_BACK_DATA);

                MessageActivity.this.overridePendingTransition(R.anim.tt_album_enter, R.anim.tt_stay);
                //addOthersPanelView.setVisibility(View.GONE);
                messageEdt.clearFocus();//切记清除焦点
                scrollToBottomListItem();
            }
            break;
            case R.id.take_camera_btn: {
                Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
                takePhotoSavePath = CommonUtil.getImageSavePath(String.valueOf(System
                        .currentTimeMillis())
                        + ".jpg");
                intent.putExtra(MediaStore.EXTRA_OUTPUT, Uri.fromFile(new File(takePhotoSavePath)));
                startActivityForResult(intent, SysConstant.CAMERA_WITH_DATA);
                //addOthersPanelView.setVisibility(View.GONE);
                messageEdt.clearFocus();//切记清除焦点
                scrollToBottomListItem();
            }
            break;
            case R.id.show_emo_btn: {
                /**yingmu 调整成键盘输出*/
                recordAudioBtn.setVisibility(View.GONE);
                keyboardInputImg.setVisibility(View.GONE);
                messageEdt.setVisibility(View.VISIBLE);
                audioInputImg.setVisibility(View.VISIBLE);
                addEmoBtn.setVisibility(View.VISIBLE);
                /**end*/
                if (keyboardHeight != 0) {
                    this.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING);
                }
                if (emoLayout.getVisibility() == View.VISIBLE) {
                    if (!messageEdt.hasFocus()) {
                        messageEdt.requestFocus();
                    }
                    inputManager.toggleSoftInputFromWindow(messageEdt.getWindowToken(), 1, 0);
                    if (keyboardHeight == 0) {
                        emoLayout.setVisibility(View.GONE);
                    }
                } else if (emoLayout.getVisibility() == View.GONE) {
                    emoLayout.setVisibility(View.VISIBLE);
                    yayaEmoGridView.setVisibility(View.VISIBLE);
                    emoRadioGroup.check(R.id.tab1);
                    emoGridView.setVisibility(View.GONE);
                    inputManager.hideSoftInputFromWindow(messageEdt.getWindowToken(), 0);
                }
                if (addOthersPanelView.getVisibility() == View.VISIBLE) {
                    addOthersPanelView.setVisibility(View.GONE);
                }
            }
            break;
            case R.id.send_message_btn: {
                logger.d("message_activity#send btn clicked");

                String content = messageEdt.getText().toString();
                logger.d("message_activity#chat content:%s", content);
                if (content.trim().equals("")) {
                    Toast.makeText(MessageActivity.this,
                            getResources().getString(R.string.message_null), Toast.LENGTH_LONG).show();
                    return;
                }
                TextMessage textMessage = TextMessage.buildForSend(content, loginUser, peerEntity);
                imService.getMessageManager().sendText(textMessage);
                messageEdt.setText("");
                pushList(textMessage);
                scrollToBottomListItem();
            }
            break;
            case R.id.voice_btn: {
                inputManager.hideSoftInputFromWindow(messageEdt.getWindowToken(), 0);
                messageEdt.setVisibility(View.GONE);
                audioInputImg.setVisibility(View.GONE);
                recordAudioBtn.setVisibility(View.VISIBLE);
                keyboardInputImg.setVisibility(View.VISIBLE);
                emoLayout.setVisibility(View.GONE);
                addOthersPanelView.setVisibility(View.GONE);
                messageEdt.setText("");
            }
            break;
            case R.id.show_keyboard_btn: {
                recordAudioBtn.setVisibility(View.GONE);
                keyboardInputImg.setVisibility(View.GONE);
                messageEdt.setVisibility(View.VISIBLE);
                audioInputImg.setVisibility(View.VISIBLE);
                addEmoBtn.setVisibility(View.VISIBLE);
            }
            break;
            case R.id.message_text:
                break;
            case R.id.tt_new_msg_tip:
            {
                scrollToBottomListItem();
                textView_new_msg_tip.setVisibility(View.GONE);
            }
            break;
        }
    }

    // 主要是录制语音的
    @Override
    public boolean onTouch(View v, MotionEvent event) {
        int id = v.getId();
        scrollToBottomListItem();
        if (id == R.id.record_voice_btn) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {

                if (AudioPlayerHandler.getInstance().isPlaying())
                    AudioPlayerHandler.getInstance().stopPlayer();
                y1 = event.getY();
                recordAudioBtn.setBackgroundResource(R.drawable.tt_pannel_btn_voiceforward_pressed);
                recordAudioBtn.setText(MessageActivity.this.getResources().getString(
                        R.string.release_to_send_voice));

                soundVolumeImg.setImageResource(R.drawable.tt_sound_volume_01);
                soundVolumeImg.setVisibility(View.VISIBLE);
                soundVolumeLayout.setBackgroundResource(R.drawable.tt_sound_volume_default_bk);
                soundVolumeDialog.show();
                audioSavePath = CommonUtil
                        .getAudioSavePath(IMLoginManager.instance().getLoginId());

                // 这个callback很蛋疼，发送消息从MotionEvent.ACTION_UP 判断
                audioRecorderInstance = new AudioRecordHandler(audioSavePath);

                audioRecorderThread = new Thread(audioRecorderInstance);
                audioRecorderInstance.setRecording(true);
                logger.d("message_activity#audio#audio record thread starts");
                audioRecorderThread.start();
            } else if (event.getAction() == MotionEvent.ACTION_MOVE) {
                y2 = event.getY();
                if (y1 - y2 > 180) {
                    soundVolumeImg.setVisibility(View.GONE);
                    soundVolumeLayout.setBackgroundResource(R.drawable.tt_sound_volume_cancel_bk);
                } else {
                    soundVolumeImg.setVisibility(View.VISIBLE);
                    soundVolumeLayout.setBackgroundResource(R.drawable.tt_sound_volume_default_bk);
                }
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                y2 = event.getY();
                if (audioRecorderInstance.isRecording()) {
                    audioRecorderInstance.setRecording(false);
                }
                if (soundVolumeDialog.isShowing()) {
                    soundVolumeDialog.dismiss();
                }
                recordAudioBtn.setBackgroundResource(R.drawable.tt_pannel_btn_voiceforward_normal);
                recordAudioBtn.setText(MessageActivity.this.getResources().getString(
                        R.string.tip_for_voice_forward));
                if (y1 - y2 <= 180) {
                    if (audioRecorderInstance.getRecordTime() >= 0.5) {
                        if (audioRecorderInstance.getRecordTime() < SysConstant.MAX_SOUND_RECORD_TIME) {
                            Message msg = uiHandler.obtainMessage();
                            msg.what = HandlerConstant.HANDLER_RECORD_FINISHED;
                            msg.obj = audioRecorderInstance.getRecordTime();
                            uiHandler.sendMessage(msg);
                        }
                    } else {
                        soundVolumeImg.setVisibility(View.GONE);
                        soundVolumeLayout
                                .setBackgroundResource(R.drawable.tt_sound_volume_short_tip_bk);
                        soundVolumeDialog.show();
                        Timer timer = new Timer();
                        timer.schedule(new TimerTask() {
                            public void run() {
                                if (soundVolumeDialog.isShowing())
                                    soundVolumeDialog.dismiss();
                                this.cancel();
                            }
                        }, 700);
                    }
                }
            }
        }
        return false;
    }

    @Override
    protected void onStop() {
        logger.d("message_activity#onStop:%s", this);

        if (null != adapter) {
            adapter.hidePopup();
        }

        AudioPlayerHandler.getInstance().clear();
        super.onStop();
    }

    @Override
    protected void onStart() {
        logger.d("message_activity#onStart:%s", this);
        super.onStart();
    }

    @Override
    public void afterTextChanged(Editable s) {
    }

    @Override
    public void beforeTextChanged(CharSequence s, int start, int count,
                                  int after) {
    }

    @Override
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        if (s.length() > 0) {
            sendBtn.setVisibility(View.VISIBLE);
            RelativeLayout.LayoutParams param = (LayoutParams) messageEdt
                    .getLayoutParams();
            param.addRule(RelativeLayout.LEFT_OF, R.id.show_emo_btn);
            addPhotoBtn.setVisibility(View.GONE);
        } else {
            addPhotoBtn.setVisibility(View.VISIBLE);
            RelativeLayout.LayoutParams param = (LayoutParams) messageEdt
                    .getLayoutParams();
            param.addRule(RelativeLayout.LEFT_OF, R.id.show_emo_btn);
            sendBtn.setVisibility(View.GONE);
        }
    }

    /**
     * @Description 滑动到列表底部
     */
    private void scrollToBottomListItem() {
        logger.d("message_activity#scrollToBottomListItem");

        // todo eric, why use the last one index + 2 can real scroll to the
        // bottom?
        ListView lv = lvPTR.getRefreshableView();
        if (lv != null) {
            lv.setSelection(adapter.getCount() + 1);
        }
        textView_new_msg_tip.setVisibility(View.GONE);
    }

    @Override
    protected void onPause() {
        logger.d("message_activity#onPause:%s", this);
        super.onPause();
    }

    @Override
    public void onAccuracyChanged(Sensor arg0, int arg1) {
    }

    @Override
    public void onSensorChanged(SensorEvent arg0) {
        try {
            if (!AudioPlayerHandler.getInstance().isPlaying()) {
                return;
            }
            float range = arg0.values[0];
            if (null != sensor && range == sensor.getMaximumRange()) {
                // 屏幕恢复亮度
                AudioPlayerHandler.getInstance().setAudioMode(AudioManager.MODE_NORMAL, this);
            } else {
                // 屏幕变黑
                AudioPlayerHandler.getInstance().setAudioMode(AudioManager.MODE_IN_CALL, this);
            }
        } catch (Exception e) {
            logger.error(e);
        }
    }

    public static Handler getUiHandler() {
        return uiHandler;
    }

    private void actFinish() {
        inputManager.hideSoftInputFromWindow(messageEdt.getWindowToken(), 0);
        IMStackManager.getStackManager().popTopActivitys(MainActivity.class);
        IMApplication.gifRunning = false;
        MessageActivity.this.finish();
    }

    private RadioGroup.OnCheckedChangeListener emoOnCheckedChangeListener = new RadioGroup.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(RadioGroup radioGroup, int id) {
            switch (id) {
                case R.id.tab2:
                    if (emoGridView.getVisibility() != View.VISIBLE) {
                        yayaEmoGridView.setVisibility(View.GONE);
                        emoGridView.setVisibility(View.VISIBLE);
                    }
                    break;
                case R.id.tab1:
                    if (yayaEmoGridView.getVisibility() != View.VISIBLE) {
                        emoGridView.setVisibility(View.GONE);
                        yayaEmoGridView.setVisibility(View.VISIBLE);
                    }
                    break;
            }
        }
    };

    private YayaEmoGridView.OnEmoGridViewItemClick yayaOnEmoGridViewItemClick = new YayaEmoGridView.OnEmoGridViewItemClick() {
        @Override
        public void onItemClick(int facesPos, int viewIndex) {
            int resId = Emoparser.getInstance(MessageActivity.this).getYayaResIdList()[facesPos];
            logger.d("message_activity#yayaEmoGridView be clicked");

            String content = Emoparser.getInstance(MessageActivity.this).getYayaIdPhraseMap()
                    .get(resId);
            if (content.equals("")) {
                Toast.makeText(MessageActivity.this,
                        getResources().getString(R.string.message_null), Toast.LENGTH_LONG).show();
                return;
            }

            TextMessage textMessage = TextMessage.buildForSend(content, loginUser, peerEntity);
            imService.getMessageManager().sendText(textMessage);
            pushList(textMessage);
            scrollToBottomListItem();
        }
    };

    private OnEmoGridViewItemClick onEmoGridViewItemClick = new OnEmoGridViewItemClick() {
        @Override
        public void onItemClick(int facesPos, int viewIndex) {
            int deleteId = (++viewIndex) * (SysConstant.pageSize - 1);
            if (deleteId > Emoparser.getInstance(MessageActivity.this).getResIdList().length) {
                deleteId = Emoparser.getInstance(MessageActivity.this).getResIdList().length;
            }
            if (deleteId == facesPos) {
                String msgContent = messageEdt.getText().toString();
                if (msgContent.isEmpty())
                    return;
                if (msgContent.contains("["))
                    msgContent = msgContent.substring(0, msgContent.lastIndexOf("["));
                messageEdt.setText(msgContent);
            } else {
                int resId = Emoparser.getInstance(MessageActivity.this).getResIdList()[facesPos];
                String pharse = Emoparser.getInstance(MessageActivity.this).getIdPhraseMap()
                        .get(resId);
                int startIndex = messageEdt.getSelectionStart();
                Editable edit = messageEdt.getEditableText();
                if (startIndex < 0 || startIndex >= edit.length()) {
                    if (null != pharse) {
                        edit.append(pharse);
                    }
                } else {
                    if (null != pharse) {
                        edit.insert(startIndex, pharse);
                    }
                }
            }
            Editable edtable = messageEdt.getText();
            int position = edtable.length();
            Selection.setSelection(edtable, position);
        }
    };

    private OnTouchListener lvPTROnTouchListener = new View.OnTouchListener() {

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                messageEdt.clearFocus();
                if (emoLayout.getVisibility() == View.VISIBLE) {
                    emoLayout.setVisibility(View.GONE);
                }

                if (addOthersPanelView.getVisibility() == View.VISIBLE) {
                    addOthersPanelView.setVisibility(View.GONE);
                }
                inputManager.hideSoftInputFromWindow(messageEdt.getWindowToken(), 0);
            }
            return false;
        }
    };

    private View.OnFocusChangeListener msgEditOnFocusChangeListener = new android.view.View.OnFocusChangeListener() {
        @Override
        public void onFocusChange(View v, boolean hasFocus) {
            if (hasFocus) {
                if (keyboardHeight == 0) {
                    addOthersPanelView.setVisibility(View.GONE);
                    emoLayout.setVisibility(View.GONE);
                } else {
                    MessageActivity.this.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING);
                    if (addOthersPanelView.getVisibility() == View.GONE) {
                        addOthersPanelView.setVisibility(View.VISIBLE);
                    }
                }
            }
        }
    };

    private ViewTreeObserver.OnGlobalLayoutListener onGlobalLayoutListener = new ViewTreeObserver.OnGlobalLayoutListener() {
        @Override
        public void onGlobalLayout() {
            Rect r = new Rect();
            baseRoot.getGlobalVisibleRect(r);
            // 进入Activity时会布局，第一次调用onGlobalLayout，先记录开始软键盘没有弹出时底部的位置
            if (rootBottom == Integer.MIN_VALUE) {
                rootBottom = r.bottom;
                return;
            }
            // adjustResize，软键盘弹出后高度会变小
            if (r.bottom < rootBottom) {
                //按照键盘高度设置表情框和发送图片按钮框的高度
                keyboardHeight = rootBottom - r.bottom;
                SystemConfigSp.instance().init(MessageActivity.this);
                SystemConfigSp.instance().setIntConfig(currentInputMethod, keyboardHeight);
                LayoutParams params = (LayoutParams) addOthersPanelView.getLayoutParams();
                params.height = keyboardHeight;
                LayoutParams params1 = (LayoutParams) emoLayout.getLayoutParams();
                params1.height = keyboardHeight;
            }
        }
    };

    private class switchInputMethodReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals("android.intent.action.INPUT_METHOD_CHANGED")) {
                currentInputMethod = Settings.Secure.getString(MessageActivity.this.getContentResolver(), Settings.Secure.DEFAULT_INPUT_METHOD);
                SystemConfigSp.instance().setStrConfig(SystemConfigSp.SysCfgDimension.DEFAULTINPUTMETHOD, currentInputMethod);
                int height =  SystemConfigSp.instance().getIntConfig(currentInputMethod);
                if(keyboardHeight!=height)
                {
                    keyboardHeight = height;
                    addOthersPanelView.setVisibility(View.GONE);
                    emoLayout.setVisibility(View.GONE);
                    MessageActivity.this.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
                    messageEdt.requestFocus();
                    if(keyboardHeight!=0 && addOthersPanelView.getLayoutParams().height!=keyboardHeight)
                    {
                        LayoutParams params = (LayoutParams) addOthersPanelView.getLayoutParams();
                        params.height = keyboardHeight;
                    }
                    if(keyboardHeight!=0 && emoLayout.getLayoutParams().height!=keyboardHeight)
                    {
                        LayoutParams params = (LayoutParams) emoLayout.getLayoutParams();
                        params.height = keyboardHeight;
                    }
                }
                else
                {
                    addOthersPanelView.setVisibility(View.VISIBLE);
                    emoLayout.setVisibility(View.VISIBLE);
                    MessageActivity.this.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING);
                    messageEdt.requestFocus();
                }
            }
        }
    }
}
