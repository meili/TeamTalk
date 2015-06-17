package com.mogujie.tt.ui.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.mogujie.tt.R;
import com.mogujie.tt.utils.ScreenUtil;

import java.util.ArrayList;

/**
 * Created by zhujian on 15/1/14.
 */
public class IMGroupAvatar extends LinearLayout {
    private String avatarUrlAppend=null;
    private int parentAvatarSize = 0;
    private int childAvatarSize = 0;
    private int childMargin = 0;
    private int avatarCount = 0;
    private int maxColumnSize = 0;
    private int rowCnt = 0;
    private int parentPadding = 0;
    private int childCorner =0;
    private int defaultChildAvatarRes = R.drawable.group_default;
    private int defaultParentAvatarBk = R.drawable.group_avatar_bk;
    private ArrayList<ImageView> mAvatarImages = new ArrayList<>();
    private ArrayList<String> mImageUrls = new ArrayList<String>();

    private static int DEFAULT_MAX_COLUMN_SIZE = 3;
    private static final int DEFAULT_PADDING_DIP = 3;
    private static final int DEFAULT_MARGIN_DIP = 2;
    private static final int DEFAULT_VIEW_SIZE_DIP = 53;
    private static int currentIndex;


    public IMGroupAvatar(Context ctx){
        this(ctx, null);
    }

    public IMGroupAvatar(Context context, AttributeSet attrs) {
        super(context, attrs);
        setBackgroundResource(defaultParentAvatarBk);
        setOrientation(VERTICAL);
        setGravity(Gravity.CENTER);
        parentPadding = ScreenUtil.instance(getContext()).dip2px(DEFAULT_PADDING_DIP);
        childMargin = ScreenUtil.instance(getContext()).dip2px(DEFAULT_MARGIN_DIP);
        parentAvatarSize = ScreenUtil.instance(getContext()).dip2px(DEFAULT_VIEW_SIZE_DIP);
        setPadding(parentPadding, parentPadding, parentPadding, parentPadding);
    }

    public void setParentPadding(int padding){
        if (padding >= 0) {
            parentPadding = ScreenUtil.instance(getContext()).dip2px(padding);
            setPadding(parentPadding, parentPadding, parentPadding, parentPadding);
        }
    }

    public void setAvatarUrlAppend(String appendStr){
        this.avatarUrlAppend=appendStr;
    }

    public void setAvatarUrls(ArrayList<String> avatarUrls){
        try {
            if (null != avatarUrls && avatarUrls.size() >= 1) {
                if (null != this.mImageUrls && this.mImageUrls.size() > 0 && this.mImageUrls.size() == avatarUrls.size()) {
                    ArrayList<String> urls = new ArrayList<String>(this.mImageUrls);
                    for (String url : avatarUrls) {
                        if (urls.contains(url)) {
                            urls.remove(url);
                        }
                    }
                    if (null != urls && urls.size() == 0) {
                        return;
                    }
                }

                removeAllViews();
                mAvatarImages.clear();
                mImageUrls.clear();
                mImageUrls.addAll(avatarUrls);
                calcParams(avatarUrls.size());
                initAvatarsLayout();
            }
        }catch (Exception e){
        }
    }

    private void calcParams(int avatarCount){
        this.avatarCount = avatarCount;
        maxColumnSize = getColumnMaxSize(avatarCount);
        if (maxColumnSize > 0){
            rowCnt = (this.avatarCount + maxColumnSize - 1 ) / maxColumnSize;
        }

        int totalWidth = getMeasuredWidth() > 0 ? getMeasuredWidth() : parentAvatarSize;
        if (maxColumnSize > 0) {
            childAvatarSize = (totalWidth - 2 * parentPadding - (maxColumnSize - 1) * childMargin) / maxColumnSize;
        }
    }

    private void initAvatarsLayout(){
        int columnSizeOfFirstRow = 0;
        columnSizeOfFirstRow = avatarCount - (rowCnt - 1) * maxColumnSize;
        currentIndex=0;
        for (int i = 0; i < rowCnt; i++){
            initOneRow(i,i == 0, i == 0 ? columnSizeOfFirstRow : maxColumnSize);
        }
    }

    private void initOneRow(int curRow,boolean isFirst, int columnSize){
        try {
            LinearLayout rowLayout = new LinearLayout(getContext());
            rowLayout.setOrientation(HORIZONTAL);
            LayoutParams params = new LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, childAvatarSize);
            if (!isFirst) {
                params.topMargin = childMargin;
            }
            addView(rowLayout, params);

            for (int i = 0; i < columnSize; i++) {
                 IMBaseImageView avatar = new IMBaseImageView(getContext());
                avatar.setScaleType(ImageView.ScaleType.CENTER_CROP);
                LayoutParams avatarParams = new LayoutParams(childAvatarSize, childAvatarSize);
                avatarParams.leftMargin = i == 0 ? 0 : childMargin;

                String avatarUrl = null;
                avatarUrl = mImageUrls.get(currentIndex);
                avatar.setImageResource(R.drawable.tt_default_user_portrait_corner);
                avatar.setCorner(ScreenUtil.instance(getContext()).dip2px(childCorner));
                avatar.setDefaultImageRes(defaultChildAvatarRes);
                avatar.setImageUrl(avatarUrl);

                rowLayout.addView(avatar, avatarParams);
                mAvatarImages.add(avatar);
                currentIndex+=1;
            }
        }catch (Exception e){

        }
    }

    private int getColumnMaxSize(int avatarCount){
        int columnSize = DEFAULT_MAX_COLUMN_SIZE;
        for (columnSize = DEFAULT_MAX_COLUMN_SIZE; columnSize > 1; columnSize--){
            int rowSize = (avatarCount + columnSize - 1) / columnSize;
            int minColumn = avatarCount - (rowSize - 1) * columnSize;
            if (columnSize == rowSize || (Math.abs(columnSize - rowSize) == 1 && Math.abs(columnSize - minColumn) <= 1)){
                return columnSize;
            }
        }
        return columnSize;
    }

    public void setChildCorner(int childCorner) {
        this.childCorner = childCorner;
    }

    public int getDefaultParentAvatarBk() {
        return defaultParentAvatarBk;
    }

    public int getChildCorner() {
        return childCorner;
    }

    public ArrayList<String>  getAvatarUrls(){
        return this.mImageUrls;
    }

    public void setChildMarginDip(int marginDip){
        childMargin =marginDip;
    }

    public void setViewSize(int viewSize){
        if (viewSize > 0) {
            parentAvatarSize = viewSize;
        }
    }

    public void setDefaultParentAvatarBk(int defaultParentAvatarBk) {
        this.defaultParentAvatarBk = defaultParentAvatarBk;
    }

    public int getDefaultChildAvatarRes() {
        return defaultChildAvatarRes;
    }

    public void setDefaultChildAvatarRes(int defaultChildAvatarRes) {
        this.defaultChildAvatarRes = defaultChildAvatarRes;
    }

    public void setMaxColumnSize(int maxColumnSize){
        DEFAULT_MAX_COLUMN_SIZE = maxColumnSize;
    }
}