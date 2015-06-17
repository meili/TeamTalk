package com.mogujie.tt.ui.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.mogujie.tt.DB.entity.GroupEntity;
import com.mogujie.tt.DB.entity.PeerEntity;
import com.mogujie.tt.DB.entity.UserEntity;
import com.mogujie.tt.R;
import com.mogujie.tt.config.DBConstant;
import com.mogujie.tt.config.SysConstant;
import com.mogujie.tt.imservice.manager.IMContactManager;
import com.mogujie.tt.imservice.service.IMService;
import com.mogujie.tt.ui.widget.IMBaseImageView;
import com.mogujie.tt.utils.IMUIHelper;
import com.mogujie.tt.utils.Logger;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;


/**
 * @YM 改造
 */
public class GroupManagerAdapter extends BaseAdapter {
	private Logger logger = Logger.getLogger(GroupManagerAdapter.class);
	private Context context;

    // 用于控制是否是删除状态，也就是那个减号是否出现
	private boolean removeState = false;
    private boolean showMinusTag = false;
    private boolean showPlusTag = false;


	private List<UserEntity> memberList = new ArrayList<>();
    private IMService imService;
    private int groupCreatorId = -1;
    private PeerEntity peerEntity;

	public GroupManagerAdapter(Context c,IMService imService,PeerEntity peerEntity) {
        memberList.clear();
        this.context = c;
		this.imService = imService;
        this.peerEntity = peerEntity;
        setData();
	}

    //todo 在选择添加人页面，currentGroupEntity 的值没有设定
	public void setData() {
        int sessionType = peerEntity.getType();
        switch (sessionType){
            case DBConstant.SESSION_TYPE_GROUP:{
               GroupEntity groupEntity =  (GroupEntity)peerEntity;
               setGroupData(groupEntity);
            }break;
            case DBConstant.SESSION_TYPE_SINGLE:{
                setSingleData((UserEntity)peerEntity);
            }break;
        }
        notifyDataSetChanged();
	}

    private void setGroupData(GroupEntity entity){
        int loginId = imService.getLoginManager().getLoginId();
        int ownerId = entity.getCreatorId();
        IMContactManager manager = imService.getContactManager();
        for(Integer memId:entity.getlistGroupMemberIds()){
           UserEntity user =  manager.findContact(memId);
           if(user!=null){
               if(ownerId == user.getPeerId()){
                   // 群主放在第一个
                   groupCreatorId =ownerId;
                   memberList.add(0, user);
               }else {
                   memberList.add(user);
               }
           }
        }
        //按钮状态的判断
        switch (entity.getGroupType()){
            case DBConstant.GROUP_TYPE_TEMP:{
                if(loginId == entity.getCreatorId()){
                    showMinusTag = true;
                    showPlusTag = true;
                }else{
                    //展示 +
                    showPlusTag = true;
                }
            }
            break;
            case DBConstant.GROUP_TYPE_NORMAL:{
                if(loginId == entity.getCreatorId()){
                    // 展示加减
                    showMinusTag = true;
                    showPlusTag = true;
                }else{
                    // 什么也不展示
                }
            }
            break;
        }
    }

    private void setSingleData(UserEntity userEntity){
        if(userEntity != null){
            memberList.add(userEntity);
            showPlusTag = true;
        }
    }

	public int getCount() {
		if (null != memberList ) {
			int memberListSize = memberList.size();
            if(showPlusTag){
                memberListSize = memberListSize +1;
            }
            // 现在的情况是有减 一定有加
            if(showMinusTag){
                memberListSize = memberListSize +1;
            }
            return memberListSize;
		}
		return 0;
	}

	public Object getItem(int position) {
		return null;
	}

	public long getItemId(int position) {
		return position;
	}


	public void removeById(int contactId) {
        for (UserEntity contact : memberList) {
            if (contact.getPeerId() == contactId) {
                memberList.remove(contact);
                break;
            }
        }
        notifyDataSetChanged();
	}

	public void add(UserEntity contact) {
		removeState = false;
		memberList.add(contact);
		notifyDataSetChanged();
	}

    public void add(List<UserEntity> list){
        removeState = false;
        // 群成员的展示没有去重，在收到IMGroupChangeMemberNotify 可能会造成重复数据
        for(UserEntity userEntity:list){
            if(!memberList.contains(userEntity)){
                memberList.add(userEntity);
            }
        }
        notifyDataSetChanged();
    }


	public View getView(int position, View convertView, ViewGroup parent) {
		logger.d("debug#getView position:%d, member size:%d", position, memberList.size());

		GroupHolder holder;
        if(convertView==null)
        {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.tt_group_manage_grid_item, null);

            holder = new GroupHolder();
            holder.imageView =  (IMBaseImageView) convertView.findViewById(R.id.grid_item_image);
            holder.userTitle = (TextView) convertView.findViewById(R.id.group_manager_user_title);
            holder.role = (ImageView)convertView.findViewById(R.id.grid_item_image_role);
            holder.deleteImg = convertView.findViewById(R.id.deleteLayout);
            holder.imageView.setDefaultImageRes(R.drawable.tt_default_user_portrait_corner);
            convertView.setTag(holder);
        }
        else
        {
            holder = (GroupHolder)convertView.getTag();
        }

        holder.role.setVisibility(View.GONE);
		if (position >= 0 && memberList.size() > position) {
			logger.d("groupmgr#in mebers area");
			final UserEntity userEntity = memberList.get(position);
			setHolder(holder, position, userEntity.getAvatar(), 0, userEntity.getMainName(), userEntity);
			
			if (holder.imageView != null) {
				holder.imageView.setOnClickListener( new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						IMUIHelper.openUserProfileActivity(context, userEntity.getPeerId());
					}
				});
			}
            if(groupCreatorId > 0 && groupCreatorId ==userEntity.getPeerId()){
                holder.role.setVisibility(View.VISIBLE);
            }

            if (removeState && userEntity.getPeerId()!= groupCreatorId) {
                holder.deleteImg.setVisibility(View.VISIBLE);
            } else {
                holder.deleteImg.setVisibility(View.INVISIBLE);
            }

		} else if (position == memberList.size() && showPlusTag) {
			logger.d("groupmgr#onAddMsg + button");
			setHolder(holder, position, null, R.drawable.tt_group_manager_add_user, "", null);
            holder.imageView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                        logger.d("groupmgr#click onAddMsg MemberButton");
                        IMUIHelper.openGroupMemberSelectActivity(context,peerEntity.getSessionKey());
                    }
            });
            holder.deleteImg.setVisibility(View.INVISIBLE);

		} else if (position == memberList.size() + 1 && showMinusTag) {
			logger.d("groupmgr#onAddMsg - button");
			setHolder(holder, position, null, R.drawable.tt_group_manager_delete_user, "", null);

            holder.imageView.setOnClickListener(new View.OnClickListener() {
                 @Override
                 public void onClick(View view) {
                     logger.d("groupmgr#click delete MemberButton");
                     toggleDeleteIcon();
                 }
            });
            holder.deleteImg.setVisibility(View.INVISIBLE);
		}
		return convertView;
	}

	private void setHolder(final GroupHolder holder, int position,
			String avatarUrl, int avatarResourceId, String name,
			UserEntity contactEntity) {
		logger.d("debug#setHolder position:%d", position);

		if (null != holder) {

			// holder.imageView.setAdjustViewBounds(false);
			// holder.imageView.setScaleType(ImageView.ScaleType.CENTER_CROP);
			if (avatarUrl != null) {
                //头像设置
                holder.imageView.setDefaultImageRes(R.drawable.tt_default_user_portrait_corner);
                holder.imageView.setCorner(8);
                holder.imageView.setImageResource(R.drawable.tt_default_user_portrait_corner);
                holder.imageView.setImageUrl(avatarUrl);

            } else {
				logger.d("groupmgr#setimageresid %d", avatarResourceId);
                holder.imageView.setImageId(0);
                holder.imageView.setImageId(avatarResourceId);
                holder.imageView.setImageUrl(avatarUrl);
			}

			holder.contactEntity = contactEntity;
			if (contactEntity != null) {
				logger.d("debug#setHolderContact name:%s", contactEntity.getMainName());

				holder.deleteImg.setOnClickListener(new OnClickListener() {

					@Override
					public void onClick(View v) {
                        if(holder.contactEntity == null){return;}
                        int userId = holder.contactEntity.getPeerId();
                        removeById(userId);
                        Set<Integer> removeMemberlist = new HashSet<>(1);
                        removeMemberlist.add(userId);
                        imService.getGroupManager().reqRemoveGroupMember(peerEntity.getPeerId(), removeMemberlist);
					}
				});
			}

			holder.userTitle.setText(name);
			holder.imageView.setVisibility(View.VISIBLE);
			holder.userTitle.setVisibility(View.VISIBLE);
		}
	}

	
	final class GroupHolder {
		IMBaseImageView imageView;
		TextView userTitle;
		View deleteImg;
		UserEntity contactEntity;
        ImageView role;
	}

    public void toggleDeleteIcon(){
        removeState = !removeState;
        notifyDataSetChanged();
    }

}
