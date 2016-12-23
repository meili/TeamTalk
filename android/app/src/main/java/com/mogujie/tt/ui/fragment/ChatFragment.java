
package com.mogujie.tt.ui.fragment;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.ContextThemeWrapper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mogujie.tt.DB.entity.GroupEntity;
import com.mogujie.tt.R;
import com.mogujie.tt.config.DBConstant;
import com.mogujie.tt.imservice.entity.RecentInfo;
import com.mogujie.tt.imservice.event.GroupEvent;
import com.mogujie.tt.imservice.event.LoginEvent;
import com.mogujie.tt.imservice.event.ReconnectEvent;
import com.mogujie.tt.imservice.event.SessionEvent;
import com.mogujie.tt.imservice.event.SocketEvent;
import com.mogujie.tt.imservice.event.UnreadEvent;
import com.mogujie.tt.imservice.event.UserInfoEvent;
import com.mogujie.tt.imservice.manager.IMLoginManager;
import com.mogujie.tt.imservice.manager.IMReconnectManager;
import com.mogujie.tt.imservice.manager.IMUnreadMsgManager;
import com.mogujie.tt.imservice.service.IMService;
import com.mogujie.tt.imservice.support.IMServiceConnector;
import com.mogujie.tt.ui.activity.MainActivity;
import com.mogujie.tt.ui.adapter.ChatAdapter;
import com.mogujie.tt.utils.IMUIHelper;
import com.mogujie.tt.utils.NetworkUtil;
import com.nostra13.universalimageloader.core.ImageLoader;
import com.nostra13.universalimageloader.core.listener.PauseOnScrollListener;

import java.util.List;

import de.greenrobot.event.EventBus;

/**
 * @Description 最近联系人Fragment页
 * @author Nana
 * @date 2014-7-24
 *
 */
public class ChatFragment extends MainFragment
        implements
        OnItemSelectedListener,
        OnItemClickListener,
        OnItemLongClickListener {

    private ChatAdapter contactAdapter;
    private ListView contactListView;
    private View curView = null;
    private View noNetworkView;
    private View noChatView;
    private ImageView notifyImage;
    private TextView  displayView;
    private ProgressBar reconnectingProgressBar;
    private IMService imService;

    //是否是手动点击重练。fasle:不显示各种弹出小气泡. true:显示小气泡直到错误出现
    private volatile boolean isManualMConnect = false;

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
    }

    private IMServiceConnector imServiceConnector = new IMServiceConnector(){

        @Override
        public void onServiceDisconnected() {
            if(EventBus.getDefault().isRegistered(ChatFragment.this)){
                EventBus.getDefault().unregister(ChatFragment.this);
            }
        }
        @Override
        public void onIMServiceConnected() {
            // TODO Auto-generated method stub
            logger.d("chatfragment#recent#onIMServiceConnected");
            imService = imServiceConnector.getIMService();
            if (imService == null) {
                // why ,some reason
                return;
            }
            // 依赖联系人回话、未读消息、用户的信息三者的状态
            onRecentContactDataReady();
            EventBus.getDefault().registerSticky(ChatFragment.this);
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        imServiceConnector.connect(getActivity());
        logger.d("chatfragment#onCreate");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle bundle) {
        logger.d("onCreateView");
        if (null != curView) {
            logger.d("curView is not null, remove it");
            ((ViewGroup) curView.getParent()).removeView(curView);
        }
        curView = inflater.inflate(R.layout.tt_fragment_chat, topContentView);
        // 多端登陆也在用这个view
        noNetworkView = curView.findViewById(R.id.layout_no_network);
        noChatView = curView.findViewById(R.id.layout_no_chat);
        reconnectingProgressBar = (ProgressBar) curView.findViewById(R.id.progressbar_reconnect);
        displayView = (TextView) curView.findViewById(R.id.disconnect_text);
        notifyImage = (ImageView) curView.findViewById(R.id.imageWifi);

        super.init(curView);
        initTitleView();// 初始化顶部view
        initContactListView(); // 初始化联系人列表视图
        showProgressBar();// 创建时没有数据，显示加载动画
        return curView;
    }
    /**
     * @Description 设置顶部按钮
     */
    private void initTitleView() {
        // 设置标题
        setTopTitleBold(getActivity().getString(R.string.chat_title));
    }
    private void initContactListView() {
        contactListView = (ListView) curView.findViewById(R.id.ContactListView);
        contactListView.setOnItemClickListener(this);
        contactListView.setOnItemLongClickListener(this);
        contactAdapter = new  ChatAdapter(getActivity());
        contactListView.setAdapter(contactAdapter);

        // this is critical, disable loading when finger sliding, otherwise
        // you'll find sliding is not very smooth
        contactListView.setOnScrollListener(new PauseOnScrollListener(ImageLoader.getInstance(),
                true, true));
    }

    @Override
    public void onStart() {
        logger.d("chatfragment#onStart");
        super.onStart();
    }

    @Override
    public void onStop() {
        logger.d("chatfragment#onStop");
        super.onStop();
    }

    @Override
    public void onPause() {
        logger.d("chatfragment#onPause");
        super.onPause();
    }

    @Override
    public void onDestroy() {
        if(EventBus.getDefault().isRegistered(ChatFragment.this)){
            EventBus.getDefault().unregister(ChatFragment.this);
        }
        imServiceConnector.disconnect(getActivity());
        super.onDestroy();
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position,
            long id) {
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
    }

    // 这个地方跳转一定要快
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {

        RecentInfo recentInfo = contactAdapter.getItem(position);
        if (recentInfo == null) {
            logger.e("recent#null recentInfo -> position:%d", position);
            return;
        }
       IMUIHelper.openChatActivity(getActivity(),recentInfo.getSessionKey());
    }

    public void onEventMainThread(SessionEvent sessionEvent){
        logger.d("chatfragment#SessionEvent# -> %s", sessionEvent);
        switch (sessionEvent){
            case RECENT_SESSION_LIST_UPDATE:
            case RECENT_SESSION_LIST_SUCCESS:
            case SET_SESSION_TOP:
                onRecentContactDataReady();
                break;
        }
    }

    public void onEventMainThread(GroupEvent event){
        switch (event.getEvent()){
            case GROUP_INFO_OK:
            case CHANGE_GROUP_MEMBER_SUCCESS:
                onRecentContactDataReady();
                searchDataReady();
                break;

            case GROUP_INFO_UPDATED:
                onRecentContactDataReady();
                searchDataReady();
                break;
            case SHIELD_GROUP_OK:
                // 更新最下栏的未读计数、更新session
                onShieldSuccess(event.getGroupEntity());
                break;
            case SHIELD_GROUP_FAIL:
            case SHIELD_GROUP_TIMEOUT:
                onShieldFail();
                break;
        }
    }

    public void onEventMainThread(UnreadEvent event){
        switch (event.event){
            case UNREAD_MSG_RECEIVED:
            case UNREAD_MSG_LIST_OK:
            case SESSION_READED_UNREAD_MSG:
                onRecentContactDataReady();
                break;
        }
    }
    public void onEventMainThread(UserInfoEvent event){
        switch (event){
            case USER_INFO_UPDATE:
            case USER_INFO_OK:
                onRecentContactDataReady();
                searchDataReady();
                break;
        }
    }

    public void onEventMainThread(LoginEvent loginEvent){
        logger.d("chatfragment#LoginEvent# -> %s", loginEvent);
        switch (loginEvent){
            case LOCAL_LOGIN_SUCCESS:
            case LOGINING: {
                logger.d("chatFragment#login#recv handleDoingLogin event");
                if (reconnectingProgressBar != null) {
                     reconnectingProgressBar.setVisibility(View.VISIBLE);
                }
            }break;

            case LOCAL_LOGIN_MSG_SERVICE:
            case LOGIN_OK: {
                isManualMConnect = false;
                logger.d("chatfragment#loginOk");
                noNetworkView.setVisibility(View.GONE);
            }break;

            case LOGIN_AUTH_FAILED:
            case LOGIN_INNER_FAILED:{
                onLoginFailure(loginEvent);
            }break;

            case PC_OFFLINE:
            case KICK_PC_SUCCESS:
                onPCLoginStatusNotify(false);
                break;

            case KICK_PC_FAILED:
                Toast.makeText(getActivity(),getString(R.string.kick_pc_failed), Toast.LENGTH_SHORT).show();
                break;
            case PC_ONLINE:
                onPCLoginStatusNotify(true);
                break;

            default: reconnectingProgressBar.setVisibility(View.GONE);
                break;
        }
    }


    public void onEventMainThread(SocketEvent socketEvent){
        switch (socketEvent){
            case MSG_SERVER_DISCONNECTED:
                handleServerDisconnected();
                break;

            case CONNECT_MSG_SERVER_FAILED:
            case REQ_MSG_SERVER_ADDRS_FAILED:
                handleServerDisconnected();
                onSocketFailure(socketEvent);break;
        }
    }

    public void onEventMainThread(ReconnectEvent reconnectEvent){
        switch (reconnectEvent){
            case DISABLE:{
                handleServerDisconnected();
            }break;
        }
    }

    private void  onLoginFailure(LoginEvent event){
        if(!isManualMConnect){return;}
        isManualMConnect = false;
        String errorTip = getString(IMUIHelper.getLoginErrorTip(event));
        logger.d("login#errorTip:%s", errorTip);
        reconnectingProgressBar.setVisibility(View.GONE);
        Toast.makeText(getActivity(), errorTip, Toast.LENGTH_SHORT).show();
    }

    private void  onSocketFailure(SocketEvent event){
        if(!isManualMConnect){return;}
        isManualMConnect = false;
        String errorTip = getString(IMUIHelper.getSocketErrorTip(event));
        logger.d("login#errorTip:%s", errorTip);
        reconnectingProgressBar.setVisibility(View.GONE);
        Toast.makeText(getActivity(), errorTip, Toast.LENGTH_SHORT).show();
    }

    // 更新页面以及 下面的未读总计数
    private void onShieldSuccess(GroupEntity entity){
        if(entity == null){
            return;}
        // 更新某个sessionId
        contactAdapter.updateRecentInfoByShield(entity);
        IMUnreadMsgManager unreadMsgManager =imService.getUnReadMsgManager();

        int totalUnreadMsgCnt = unreadMsgManager.getTotalUnreadCount();
        logger.d("unread#total cnt %d", totalUnreadMsgCnt);
        ((MainActivity) getActivity()).setUnreadMessageCnt(totalUnreadMsgCnt);
    }

    private void onShieldFail(){
       Toast.makeText(getActivity(), R.string.req_msg_failed, Toast.LENGTH_SHORT).show();
    }

    /**搜索数据OK
     * 群组数据与 user数据都已经完毕
     * */
    public void searchDataReady(){
        if (imService.getContactManager().isUserDataReady() &&
                imService.getGroupManager().isGroupReady()) {
            showSearchFrameLayout();
        }
    }

    /**
     * 多端，PC端在线状态通知
     * @param isOnline
     */
    public void onPCLoginStatusNotify(boolean isOnline){
        logger.d("chatfragment#onPCLoginStatusNotify");
        if(isOnline){
            reconnectingProgressBar.setVisibility(View.GONE);
            noNetworkView.setVisibility(View.VISIBLE);
            notifyImage.setImageResource(R.drawable.pc_notify);
            displayView.setText(R.string.pc_status_notify);
            /**添加踢出事件*/
            noNetworkView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                   reconnectingProgressBar.setVisibility(View.VISIBLE);
                   imService.getLoginManager().reqKickPCClient();
                }
            });
        }else{
            noNetworkView.setVisibility(View.GONE);
        }
    }

    private void handleServerDisconnected() {
        logger.d("chatfragment#handleServerDisconnected");

        if (reconnectingProgressBar != null) {
            reconnectingProgressBar.setVisibility(View.GONE);
        }

        if (noNetworkView != null) {
            notifyImage.setImageResource(R.drawable.warning);
            noNetworkView.setVisibility(View.VISIBLE);
            if(imService != null){
                if(imService.getLoginManager().isKickout()){
                    displayView.setText(R.string.disconnect_kickout);
                }else{
                    displayView.setText(R.string.no_network);
                }
            }
            /**重连【断线、被其他移动端挤掉】*/
            noNetworkView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    logger.d("chatFragment#noNetworkView clicked");
                    IMReconnectManager manager = imService.getReconnectManager();
                    if(NetworkUtil.isNetWorkAvalible(getActivity())){
                        isManualMConnect = true;
                        IMLoginManager.instance().relogin();
                    }else{
                        Toast.makeText(getActivity(), R.string.no_network_toast, Toast.LENGTH_SHORT).show();
                        return;
                    }
                    reconnectingProgressBar.setVisibility(View.VISIBLE);
                }
            });
        }
    }

    /**
     * 这个处理有点过于粗暴, 最近联系人的信息
     */
    private void onRecentContactDataReady() {
        boolean isUserData = imService.getContactManager().isUserDataReady();
        boolean isSessionData = imService.getSessionManager().isSessionListReady();
        boolean isGroupData =  imService.getGroupManager().isGroupReady();

        if ( !(isUserData&&isSessionData&&isGroupData)) {
            return;
        }
        IMUnreadMsgManager unreadMsgManager =imService.getUnReadMsgManager();

        int totalUnreadMsgCnt = unreadMsgManager.getTotalUnreadCount();
        logger.d("unread#total cnt %d", totalUnreadMsgCnt);
        ((MainActivity) getActivity()).setUnreadMessageCnt(totalUnreadMsgCnt);

        List<RecentInfo> recentSessionList = imService.getSessionManager().getRecentListInfo();

        setNoChatView(recentSessionList);
        contactAdapter.setData(recentSessionList);
        hideProgressBar();
        showSearchFrameLayout();
    }

    private void setNoChatView(List<RecentInfo> recentSessionList)
    {
        if(recentSessionList.size()==0)
        {
            noChatView.setVisibility(View.VISIBLE);
        }
        else
        {
            noChatView.setVisibility(View.GONE);
        }
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> parent, View view,
            int position, long id) {

        RecentInfo recentInfo = contactAdapter.getItem(position);
        if (recentInfo == null) {
            logger.e("recent#onItemLongClick null recentInfo -> position:%d", position);
            return false;
        }
        if (recentInfo.getSessionType() == DBConstant.SESSION_TYPE_SINGLE) {
            handleContactItemLongClick(getActivity(),recentInfo);
        } else {
            handleGroupItemLongClick(getActivity(),recentInfo);
        }
        return true;
    }

    private void handleContactItemLongClick(final Context ctx, final RecentInfo recentInfo) {

        AlertDialog.Builder builder = new AlertDialog.Builder(new ContextThemeWrapper(ctx, android.R.style.Theme_Holo_Light_Dialog));
        builder.setTitle(recentInfo.getName());
        final boolean isTop = imService.getConfigSp().isTopSession(recentInfo.getSessionKey());

        int topMessageRes = isTop?R.string.cancel_top_message:R.string.top_message;
        String[] items = new String[]{ctx.getString(R.string.check_profile),
                ctx.getString(R.string.delete_session),
                ctx.getString(topMessageRes)};

        builder.setItems(items, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case 0 :
                        IMUIHelper.openUserProfileActivity(ctx, recentInfo.getPeerId());
                        break;
                    case 1 :
                        imService.getSessionManager().reqRemoveSession(recentInfo);
                        break;
                    case 2:{
                         imService.getConfigSp().setSessionTop(recentInfo.getSessionKey(),!isTop);
                    }break;
                }
            }
        });
        AlertDialog alertDialog = builder.create();
        alertDialog.setCanceledOnTouchOutside(true);
        alertDialog.show();
    }

    // 现在只有群组存在免打扰的
    private void handleGroupItemLongClick(final Context ctx, final RecentInfo recentInfo) {

        AlertDialog.Builder builder = new AlertDialog.Builder(new ContextThemeWrapper(ctx, android.R.style.Theme_Holo_Light_Dialog));
        builder.setTitle(recentInfo.getName());

        final boolean isTop = imService.getConfigSp().isTopSession(recentInfo.getSessionKey());
        final boolean isForbidden = recentInfo.isForbidden();
        int topMessageRes = isTop?R.string.cancel_top_message:R.string.top_message;
        int forbidMessageRes =isForbidden?R.string.cancel_forbid_group_message:R.string.forbid_group_message;

        String[] items = new String[]{ctx.getString(R.string.delete_session),ctx.getString(topMessageRes),ctx.getString(forbidMessageRes)};

        builder.setItems(items, new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case 0 :
                        imService.getSessionManager().reqRemoveSession(recentInfo);
                        break;
                    case 1:{
                        imService.getConfigSp().setSessionTop(recentInfo.getSessionKey(),!isTop);
                    }break;
                    case 2:{
                        // 底层成功会事件通知
                        int shieldType = isForbidden?DBConstant.GROUP_STATUS_ONLINE:DBConstant.GROUP_STATUS_SHIELD;
                        imService.getGroupManager().reqShieldGroup(recentInfo.getPeerId(),shieldType);
                    }
                    break;
                }
            }
        });
        AlertDialog alertDialog = builder.create();
        alertDialog.setCanceledOnTouchOutside(true);
        alertDialog.show();
    }

    @Override
    protected void initHandler() {
        // TODO Auto-generated method stub
    }

    /**
     * 滚动到有未读消息的第一个联系人
     * 这个还是可行的
     */
    public void scrollToUnreadPosition() {
      if (contactListView != null) {
         int currentPosition =  contactListView.getFirstVisiblePosition();
         int needPosition = contactAdapter.getUnreadPositionOnView(currentPosition);
          // 下面这个不管用!!
         //contactListView.smoothScrollToPosition(needPosition);
          contactListView.setSelection(needPosition);
      }
    }
}
