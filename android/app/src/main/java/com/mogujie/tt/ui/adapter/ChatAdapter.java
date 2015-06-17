package com.mogujie.tt.ui.adapter;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mogujie.tools.ScreenTools;
import com.mogujie.tt.config.DBConstant;
import com.mogujie.tt.DB.entity.GroupEntity;
import com.mogujie.tt.R;
import com.mogujie.tt.config.SysConstant;
import com.mogujie.tt.imservice.entity.RecentInfo;
import com.mogujie.tt.utils.DateUtil;
import com.mogujie.tt.utils.Logger;
import com.mogujie.tt.ui.widget.IMBaseImageView;
import com.mogujie.tt.ui.widget.IMGroupAvatar;

import java.util.ArrayList;
import java.util.List;

/**
 * @Description 联系人列表适配器
 */
@SuppressLint("ResourceAsColor")
public class ChatAdapter extends BaseAdapter {
	private LayoutInflater mInflater = null;
	private List<RecentInfo> recentSessionList = new ArrayList<>();
	private Logger logger = Logger.getLogger(ChatAdapter.class);

    private static final int CONTACT_TYPE_INVALID = 0;
    private static final int CONTACT_TYPE_USER=1;
    private static final int CONTACT_TYPE_GROUP =2;

	public ChatAdapter(Context context) {
		this.mInflater = LayoutInflater.from(context);
	}

	@Override
	public int getCount() {
		return recentSessionList.size();
	}

		@Override
		public RecentInfo getItem(int position) {
			logger.d("recent#getItem position:%d", position);
			if (position >= recentSessionList.size() || position < 0) {
				return null;
			}
			return recentSessionList.get(position);
		}

    /**更新单个RecentInfo 屏蔽群组信息*/
    public void updateRecentInfoByShield(GroupEntity entity){
        String sessionKey = entity.getSessionKey();
        for(RecentInfo recentInfo:recentSessionList){
            if(recentInfo.getSessionKey().equals(sessionKey)){
                int status = entity.getStatus();
                boolean isFor = status==DBConstant.GROUP_STATUS_SHIELD;
                recentInfo.setForbidden(isFor);
                notifyDataSetChanged();
                break;
            }
        }
    }

    /**置顶状态的更新  not use now*/
    public void updateRecentInfoByTop(String sessionKey,boolean isTop){
        for(RecentInfo recentInfo:recentSessionList){
            if(recentInfo.getSessionKey().equals(sessionKey)){
                recentInfo.setTop(isTop);
                notifyDataSetChanged();
                break;
            }
        }
    }


    public int getUnreadPositionOnView(int currentPostion){
        int nextIndex = currentPostion +1;
        int sum =getCount();
        if(nextIndex > sum){
            currentPostion = 0;
        }
        /**从当前点到末尾*/
        for(int index=nextIndex;index < sum;index++){
          int unCnt =   recentSessionList.get(index).getUnReadCnt();
          if(unCnt > 0){
              return index;
          }
        }
        /**从末尾到当前点*/
        for(int index =0;index < currentPostion;index++){
            int unCnt =   recentSessionList.get(index).getUnReadCnt();
            if(unCnt > 0){
                return index;
            }
        }
        //最后返回到最上面
        return 0;
    }



	@Override
	public long getItemId(int position) {
		return position;
	}

    /**
     * 用户HOLDER
     */
	private final class ContactViewHolder extends ContactHolderBase {
		public IMBaseImageView avatar;
	}

    /**
     * 基本HOLDER
     */
    public static class ContactHolderBase{
        public TextView uname;
        public TextView lastContent;
        public TextView lastTime;
        public TextView msgCount;
        public ImageView noDisturb;
    }

    /**
     * 群组HOLDER
     */
    private final static class GroupViewHolder extends ContactHolderBase{
        public IMGroupAvatar avatarLayout;
    }

    private View renderUser(int position,View convertView, ViewGroup parent){
        RecentInfo recentInfo = recentSessionList.get(position);
        ContactViewHolder holder;
        if (null == convertView) {
            convertView = mInflater.inflate(R.layout.tt_item_chat,parent,false);
             holder = new ContactViewHolder();
            holder.avatar = (IMBaseImageView) convertView.findViewById(R.id.contact_portrait);
            holder.uname = (TextView) convertView.findViewById(R.id.shop_name);
            holder.lastContent = (TextView) convertView.findViewById(R.id.message_body);
            holder.lastTime = (TextView) convertView.findViewById(R.id.message_time);
            holder.msgCount = (TextView) convertView.findViewById(R.id.message_count_notify);
            holder.noDisturb = (ImageView)convertView.findViewById(R.id.message_time_no_disturb_view);
            holder.avatar.setImageResource(R.drawable.tt_default_user_portrait_corner);
            convertView.setTag(holder);
        }else{
            holder = (ContactViewHolder)convertView.getTag();
        }

        if(recentInfo.isTop()){
            // todo   R.color.top_session_background
            convertView.setBackgroundColor(Color.parseColor("#f4f4f4f4"));
        }else{
            convertView.setBackgroundColor(Color.WHITE);
        }

        handleCommonContact(holder,recentInfo);
        return convertView;
    }

    private View renderGroup(int position,View convertView, ViewGroup parent){
        RecentInfo recentInfo = recentSessionList.get(position);
        GroupViewHolder holder;
        if (null == convertView) {
            convertView = mInflater.inflate(R.layout.tt_item_chat_group, parent,false);
            holder = new GroupViewHolder();
            holder.avatarLayout = (IMGroupAvatar) convertView.findViewById(R.id.contact_portrait);
            holder.uname = (TextView) convertView.findViewById(R.id.shop_name);
            holder.lastContent = (TextView) convertView.findViewById(R.id.message_body);
            holder.lastTime = (TextView) convertView.findViewById(R.id.message_time);
            holder.msgCount = (TextView) convertView.findViewById(R.id.message_count_notify);
            holder.noDisturb = (ImageView)convertView.findViewById(R.id.message_time_no_disturb_view);
            convertView.setTag(holder);
        }else{
            holder = (GroupViewHolder)convertView.getTag();
        }

        if(recentInfo.isTop()){
            // todo   R.color.top_session_background
            convertView.setBackgroundColor(Color.parseColor("#f4f4f4f4"));
        }else{
            convertView.setBackgroundColor(Color.WHITE);
        }

        /**群屏蔽的设定*/
        if(recentInfo.isForbidden())
        {
            holder.noDisturb.setVisibility(View.VISIBLE);
        }
        else
        {
            holder.noDisturb.setVisibility(View.GONE);
        }

        handleGroupContact( holder,recentInfo);
        return convertView;
    }


    //yingmu base-adapter-helper 了解一下
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		//		logger.d("recent#getview position:%d", position);
		try {
            final int type = getItemViewType(position);
			ContactHolderBase holder = null;

            switch (type){
                case CONTACT_TYPE_USER:
                    convertView = renderUser(position,convertView,parent);
                    break;
                case CONTACT_TYPE_GROUP:
                    convertView =renderGroup(position,convertView,parent);
                    break;
            }
			return convertView;
		} catch (Exception e) {
			logger.e(e.toString());
			return null;
		}
	}

    @Override
    public int getViewTypeCount() {
        return 3;
    }

    @Override
    public int getItemViewType(int position) {
        try {
            if (position >= recentSessionList.size()) {
                return CONTACT_TYPE_INVALID;
            }
            RecentInfo recentInfo = recentSessionList.get(position);
            if (recentInfo.getSessionType()==DBConstant.SESSION_TYPE_SINGLE){
                return CONTACT_TYPE_USER;
            }else if(recentInfo.getSessionType()==DBConstant.SESSION_TYPE_GROUP){
                return CONTACT_TYPE_GROUP;
            }else{
                return CONTACT_TYPE_INVALID;
            }
        }catch (Exception e){
            logger.e(e.toString());
            return CONTACT_TYPE_INVALID;
        }
    }

	public void setData(List<RecentInfo> recentSessionList) {
		logger.d("recent#set New recent session list");
		logger.d("recent#notifyDataSetChanged");
        this.recentSessionList = recentSessionList;
        notifyDataSetChanged();
	}

    private void handleCommonContact(ContactViewHolder contactViewHolder,RecentInfo recentInfo)
    {
        String avatarUrl = null;
        String userName = "";
        String lastContent = "";
        String lastTime = "";
        int unReadCount = 0;
        int sessionType = DBConstant.SESSION_TYPE_SINGLE;

        userName = recentInfo.getName();
        lastContent = recentInfo.getLatestMsgData();
        // todo 是不是每次都需要计算
        lastTime = DateUtil.getSessionTime(recentInfo.getUpdateTime());
        unReadCount = recentInfo.getUnReadCnt();
        if(null!=recentInfo.getAvatar()&&recentInfo.getAvatar().size()>0)
        {
            avatarUrl = recentInfo.getAvatar().get(0);

        }
        // 设置未读消息计数
        if (unReadCount > 0) {
            String strCountString=String.valueOf(unReadCount);
            if (unReadCount>99) {
                strCountString = "99+";
            }
            contactViewHolder.msgCount.setVisibility(View.VISIBLE);
            contactViewHolder.msgCount.setText(strCountString);
        } else {
            contactViewHolder.msgCount.setVisibility(View.GONE);
        }
        //头像设置
        contactViewHolder.avatar.setDefaultImageRes(R.drawable.tt_default_user_portrait_corner);
        contactViewHolder.avatar.setCorner(8);
        contactViewHolder.avatar.setImageUrl(avatarUrl);
        // 设置其它信息
        contactViewHolder.uname.setText(userName);
        contactViewHolder.lastContent.setText(lastContent);
        contactViewHolder.lastTime.setText(lastTime);

    }

    private void handleGroupContact(GroupViewHolder groupViewHolder,RecentInfo recentInfo)
    {
        String avatarUrl = null;
        String userName = "";
        String lastContent = "";
        String lastTime = "";
        int unReadCount = 0;
        int sessionType = DBConstant.SESSION_TYPE_SINGLE;

        userName = recentInfo.getName();
        lastContent = recentInfo.getLatestMsgData();
        // todo 是不是每次都需要计算
        lastTime = DateUtil.getSessionTime(recentInfo.getUpdateTime());
        unReadCount = recentInfo.getUnReadCnt();
//        sessionType = recentInfo.getSessionType();
        // 设置未读消息计数 只有群组有的

        if (unReadCount > 0) {
            if(recentInfo.isForbidden())
            {
                groupViewHolder.msgCount.setBackgroundResource(R.drawable.tt_message_botify_no_disturb);
                groupViewHolder.msgCount.setVisibility(View.VISIBLE);
                groupViewHolder.msgCount.setText("");
                ((RelativeLayout.LayoutParams)groupViewHolder.msgCount.getLayoutParams()).leftMargin=ScreenTools.instance(this.mInflater.getContext()).dip2px(-7);
                ((RelativeLayout.LayoutParams)groupViewHolder.msgCount.getLayoutParams()).topMargin=ScreenTools.instance(this.mInflater.getContext()).dip2px(6);
                groupViewHolder.msgCount.getLayoutParams().width = ScreenTools.instance(this.mInflater.getContext()).dip2px(10);
                groupViewHolder.msgCount.getLayoutParams().height = ScreenTools.instance(this.mInflater.getContext()).dip2px(10);

            }
            else
            {
                groupViewHolder.msgCount.setBackgroundResource(R.drawable.tt_message_notify);
                groupViewHolder.msgCount.setVisibility(View.VISIBLE);
                ((RelativeLayout.LayoutParams)groupViewHolder.msgCount.getLayoutParams()).leftMargin=ScreenTools.instance(this.mInflater.getContext()).dip2px(-10);
                ((RelativeLayout.LayoutParams)groupViewHolder.msgCount.getLayoutParams()).topMargin=ScreenTools.instance(this.mInflater.getContext()).dip2px(3);
                groupViewHolder.msgCount.getLayoutParams().width = RelativeLayout.LayoutParams.WRAP_CONTENT;
                groupViewHolder.msgCount.getLayoutParams().height = RelativeLayout.LayoutParams.WRAP_CONTENT;
                groupViewHolder.msgCount.setPadding(ScreenTools.instance(this.mInflater.getContext()).dip2px(3),0,ScreenTools.instance(this.mInflater.getContext()).dip2px(3),0);

                String strCountString=String.valueOf(unReadCount);
                if (unReadCount>99) {
                    strCountString = "99+";
                }
                groupViewHolder.msgCount.setVisibility(View.VISIBLE);
                groupViewHolder.msgCount.setText(strCountString);
            }

        } else {
            groupViewHolder.msgCount.setVisibility(View.GONE);
        }

        //头像设置
        setGroupAvatar(groupViewHolder, recentInfo.getAvatar());
        // 设置其它信息
        groupViewHolder.uname.setText(userName);
        groupViewHolder.lastContent.setText(lastContent);
        groupViewHolder.lastTime.setText(lastTime);
    }

    /**
     * 设置群头像
     * @param holder
     * @param avatarUrlList
     */
    private void setGroupAvatar(GroupViewHolder holder,List<String> avatarUrlList){
        try {
        if (null == avatarUrlList) {
                return;
            }
            holder.avatarLayout.setAvatarUrlAppend(SysConstant.AVATAR_APPEND_32);
            holder.avatarLayout.setChildCorner(3);
            if (null != avatarUrlList) {
                holder.avatarLayout.setAvatarUrls(new ArrayList<String>(avatarUrlList));
            }
        }catch (Exception e){
            logger.e(e.toString());
        }

    }

}
