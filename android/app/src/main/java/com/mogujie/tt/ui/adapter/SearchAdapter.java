package com.mogujie.tt.ui.adapter;

import android.content.Context;
import android.content.Intent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.mogujie.tt.DB.entity.DepartmentEntity;
import com.mogujie.tt.DB.entity.GroupEntity;
import com.mogujie.tt.DB.entity.UserEntity;
import com.mogujie.tt.R;
import com.mogujie.tt.config.DBConstant;
import com.mogujie.tt.config.IntentConstant;
import com.mogujie.tt.config.SysConstant;
import com.mogujie.tt.imservice.service.IMService;
import com.mogujie.tt.ui.activity.MainActivity;
import com.mogujie.tt.ui.widget.IMBaseImageView;
import com.mogujie.tt.ui.widget.IMGroupAvatar;
import com.mogujie.tt.utils.IMUIHelper;
import com.mogujie.tt.utils.Logger;
import com.mogujie.tt.utils.ScreenUtil;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * @author : yingmu on 15-3-18.
 * @email : yingmu@mogujie.com.
 *
 * 列表的顺序是： 用户-->群组-->部门
 */
public class SearchAdapter extends BaseAdapter implements
        AdapterView.OnItemClickListener,
        AdapterView.OnItemLongClickListener{

    private Logger logger = Logger.getLogger(SearchAdapter.class);

    private List<UserEntity>  userList = new ArrayList<>();
    private List<DepartmentEntity> deptList = new ArrayList<>();
    private List<GroupEntity> groupList = new ArrayList<>();

    private String searchKey;
    private Context ctx;
    private IMService imService;
    public SearchAdapter(Context context,IMService pimService){
        this.ctx = context;
        this.imService = pimService;
    }

    public void clear(){
        this.userList.clear();
        this.groupList.clear();
        this.deptList.clear();
        notifyDataSetChanged();
    }

    public void putUserList(List<UserEntity> pUserList){
        this.userList.clear();
        if(pUserList == null || pUserList.size() <=0){
            return;
        }
        this.userList = pUserList;
    }

    public void putGroupList(List<GroupEntity> pGroupList){
        this.groupList.clear();
        if(pGroupList == null || pGroupList.size() <=0){
            return;
        }
        this.groupList = pGroupList;
    }

    public void putDeptList(List<DepartmentEntity> pDeptList){
        this.deptList.clear();
        if(pDeptList == null || pDeptList.size() <=0){
            return;
        }
        this.deptList = pDeptList;
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        Object object =  getItem(position);
        if(object instanceof UserEntity){
            UserEntity userEntity = (UserEntity) object;
            IMUIHelper.openChatActivity(ctx, userEntity.getSessionKey());
        }else if(object instanceof GroupEntity){
            GroupEntity groupEntity = (GroupEntity) object;
            IMUIHelper.openChatActivity(ctx, groupEntity.getSessionKey());
        }else if(object instanceof  DepartmentEntity){
            // 定位
            DepartmentEntity department = (DepartmentEntity) object;
            locateDepartment(ctx, department);
        }else{

        }
    }

    private void locateDepartment(Context ctx, DepartmentEntity department) {
        Intent intent = new Intent(ctx, MainActivity.class);
        intent.putExtra(IntentConstant.KEY_LOCATE_DEPARTMENT, department.getDepartId());
        ctx.startActivity(intent);
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
        Object object =  getItem(position);
        if(object instanceof UserEntity){
            UserEntity userEntity = (UserEntity) object;
            IMUIHelper.handleContactItemLongClick(userEntity, ctx);
        }else{
        }
        return true;
    }

    @Override
    public int getItemViewType(int position) {
        // 根据entity的类型进行判断，  或者根据长度判断
        int userSize = userList==null?0:userList.size();
        int groupSize = groupList==null?0:groupList.size();
        int deptSize = deptList==null?0:deptList.size();

        if(position < userSize){
            return SearchType.USER.ordinal();
        }else if(position < userSize + groupSize){
            return SearchType.GROUP.ordinal();
        }else{
            return SearchType.DEPT.ordinal();
        }
    }

    @Override
    public int getViewTypeCount() {
        return SearchType.values().length;
    }

    @Override
    public int getCount() {
        // todo  Elegant code
        int groupSize = groupList==null?0:groupList.size();
        int userSize = userList==null?0:userList.size();
        int deptSize = deptList==null?0:deptList.size();
        int sum = groupSize + userSize + deptSize;
        return sum;
    }

    @Override
    public Object getItem(int position) {
        int typeIndex =  getItemViewType(position);
        SearchType renderType = SearchType.values()[typeIndex];
        switch (renderType){
            case USER:{
                return userList.get(position);
            }
            case GROUP:{
                int userSize = userList==null?0:userList.size();
                int realIndex = position - userSize;
                if(realIndex < 0){
                    throw new IllegalArgumentException("SearchAdapter#getItem#group类型判断错误");
                }
                return groupList.get(realIndex);
            }
            case DEPT:{
                int userSize = userList==null?0:userList.size();
                int groupSize = groupList==null?0:groupList.size();

                int realIndex = position - userSize - groupSize;
                if(realIndex < 0){
                    throw new IllegalArgumentException("SearchAdapter#getItem#group类型判断错误");
                }
                return deptList.get(realIndex);
            }

            default:
                throw new IllegalArgumentException("SearchAdapter#getItem#不存在的类型" + renderType.name());
        }
    }


    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        int typeIndex =  getItemViewType(position);
        SearchType renderType = SearchType.values()[typeIndex];
        View view = null;
        switch (renderType){
            case USER:{
                view = renderUser(position,convertView,parent);
            }
            break;
            case GROUP:{
                view = renderGroup(position,convertView,parent);
            }
            break;
            case DEPT:{
                view = renderDept(position, convertView, parent);
            }
            break;
        }
        return view;
    }

    public View renderUser(int position, View view, ViewGroup parent){
        UserHolder userHolder = null;
        UserEntity  userEntity= (UserEntity)getItem(position);
        if(userEntity == null){
            logger.e("SearchAdapter#renderUser#userEntity is null!position:%d",position);
            return null;
        }
        if (view == null) {
            userHolder = new UserHolder();
            view = LayoutInflater.from(ctx).inflate(R.layout.tt_item_contact, parent,false);
            userHolder.nameView = (TextView) view.findViewById(R.id.contact_item_title);
            userHolder.realNameView = (TextView) view.findViewById(R.id.contact_realname_title);
            userHolder.sectionView = (TextView) view.findViewById(R.id.contact_category_title);
            userHolder.avatar = (IMBaseImageView)view.findViewById(R.id.contact_portrait);
            userHolder.divider = view.findViewById(R.id.contact_divider);
            view.setTag(userHolder);
        } else {
            userHolder = (UserHolder) view.getTag();
        }

        IMUIHelper.setTextHilighted(userHolder.nameView,userEntity.getMainName(),userEntity.getSearchElement());
        //userHolder.nameView.setText(userEntity.getNickName());

        userHolder.avatar.setImageResource(R.drawable.tt_default_user_portrait_corner);
        userHolder.divider.setVisibility(View.VISIBLE);

        // 分栏显示“联系人”
        if (position == 0) {
            userHolder.sectionView.setVisibility(View.VISIBLE);
            userHolder.sectionView.setText(ctx.getString(R.string.contact));

            //分栏已经显示，最上面的分割线不用显示
            userHolder.divider.setVisibility(View.GONE);
        }else{
            userHolder.sectionView.setVisibility(View.GONE);
            userHolder.divider.setVisibility(View.VISIBLE);
        }

        userHolder.avatar.setDefaultImageRes(R.drawable.tt_default_user_portrait_corner);
        userHolder.avatar.setCorner(0);
        userHolder.avatar.setImageUrl(userEntity.getAvatar());

        userHolder.realNameView.setText(userEntity.getRealName());
        userHolder.realNameView.setVisibility(View.GONE);
        return view;
    }

    public View renderGroup(int position, View view, ViewGroup parent){
        GroupHolder groupHolder = null;
        GroupEntity groupEntity = (GroupEntity) getItem(position);
        if(groupEntity == null){
            logger.e("SearchAdapter#renderGroup#groupEntity is null!position:%d",position);
            return null;
        }
        if (view == null) {
            groupHolder = new GroupHolder();
            view = LayoutInflater.from(ctx).inflate(R.layout.tt_item_contact_group, parent,false);
            groupHolder.nameView = (TextView) view.findViewById(R.id.contact_item_title);
            groupHolder.sectionView = (TextView) view.findViewById(R.id.contact_category_title);
            groupHolder.avatar = (IMGroupAvatar)view.findViewById(R.id.contact_portrait);
            groupHolder.divider = view.findViewById(R.id.contact_divider);
            view.setTag(groupHolder);
        } else {
            groupHolder = (GroupHolder) view.getTag();
        }

      //groupHolder.nameView.setText(groupEntity.getGroupName());
      IMUIHelper.setTextHilighted(groupHolder.nameView,groupEntity.getMainName(),groupEntity.getSearchElement());

        groupHolder.sectionView.setVisibility(View.GONE);
        // 分割线的处理【位于控件的最上面】
        groupHolder.divider.setVisibility(View.VISIBLE);

        // 分栏显示“群或讨论组”
        int userSize = userList==null?0:userList.size();
        if (position == userSize) {
            groupHolder.sectionView.setVisibility(View.VISIBLE);
            groupHolder.sectionView.setText(ctx.getString(R.string.fixed_group_or_temp_group));
            //分栏已经显示，最上面的分割线不用显示
            groupHolder.divider.setVisibility(View.GONE);
        }else{
            groupHolder.sectionView.setVisibility(View.GONE);
        }

        groupHolder.avatar.setVisibility(View.VISIBLE);
        List<String> avatarUrlList = new ArrayList<>();
        Set<Integer> userIds = groupEntity.getlistGroupMemberIds();
        int i = 0;
        for(Integer buddyId:userIds){
            UserEntity entity = imService.getContactManager().findContact(buddyId);
            if (entity == null) {
                //logger.d("已经离职。userId:%d", buddyId);
                continue;
            }
            avatarUrlList.add(entity.getAvatar());
            if (i >= 3) {
                break;
            }
            i++;
        }
        setGroupAvatar(groupHolder.avatar,avatarUrlList);
        return view;
    }

    public View renderDept(int position, View view, ViewGroup parent){
        DeptHolder deptHolder = null;
        DepartmentEntity  deptEntity= (DepartmentEntity)getItem(position);
        if(deptEntity == null){
            logger.e("SearchAdapter#renderDept#deptEntity is null!position:%d",position);
            return null;
        }
        if (view == null) {
            deptHolder = new DeptHolder();
            view = LayoutInflater.from(ctx).inflate(R.layout.tt_item_contact, parent,false);
            deptHolder.avatar = (IMBaseImageView)view.findViewById(R.id.contact_portrait);
            deptHolder.nameView = (TextView) view.findViewById(R.id.contact_item_title);
            deptHolder.sectionView = (TextView) view.findViewById(R.id.contact_category_title);
            deptHolder.divider = view.findViewById(R.id.contact_divider);
            view.setTag(deptHolder);
        } else {
            deptHolder = (DeptHolder) view.getTag();
        }
        deptHolder.avatar.setVisibility(View.INVISIBLE);
        //deptHolder.nameView.setText(deptEntity.getDepartName());
        IMUIHelper.setTextHilighted(deptHolder.nameView,deptEntity.getDepartName(),deptEntity.getSearchElement());
        deptHolder.divider.setVisibility(View.VISIBLE);

        // 分栏信息的展示 可以保存结果，优化
        int groupSize = groupList==null?0:groupList.size();
        int userSize = userList==null?0:userList.size();
        int realIndex = position - groupSize - userSize;
        if (realIndex == 0) {
            deptHolder.sectionView.setVisibility(View.VISIBLE);
            deptHolder.sectionView.setText(ctx.getString(R.string.department));

            //分栏已经显示，最上面的分割线不用显示
            deptHolder.divider.setVisibility(View.GONE);
        }else{
            deptHolder.sectionView.setVisibility(View.GONE);
        }
        return view;
    }


    /**
     *  与contactAdapter 有公用的地方，可以抽取到IMUIHelper
     * 设置群头像
     * @param avatar
     * @param avatarUrlList
     */
    private void setGroupAvatar(IMGroupAvatar avatar,List<String> avatarUrlList){
        try {
            avatar.setViewSize(ScreenUtil.instance(ctx).dip2px(38));
            avatar.setChildCorner(2);
            avatar.setAvatarUrlAppend(SysConstant.AVATAR_APPEND_32);
            avatar.setParentPadding(3);
            avatar.setAvatarUrls((ArrayList<String>) avatarUrlList);
        }catch (Exception e){
            logger.e(e.toString());
        }
    }



    // 将分割线放在上面，利于判断
    public static class UserHolder {
        View divider;
        TextView sectionView;
        TextView nameView;
        TextView realNameView;
        IMBaseImageView avatar;
    }

    public static class DeptHolder {
        View divider;
        TextView sectionView;
        TextView nameView;
        IMBaseImageView avatar;
    }

    public static class GroupHolder {
        View divider;
        TextView sectionView;
        TextView nameView;
        IMGroupAvatar avatar;
    }

    private enum SearchType{
        USER,
        GROUP,
        DEPT,
        ILLEGAL
    }

    /**---------------------------set/get--------------------------*/
    public String getSearchKey() {
        return searchKey;
    }

    public void setSearchKey(String searchKey) {
        this.searchKey = searchKey;
    }
}
