package com.mogujie.tt.ui.activity;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.view.ContextThemeWrapper;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mogujie.tt.DB.sp.LoginSp;
import com.mogujie.tt.DB.sp.SystemConfigSp;
import com.mogujie.tt.R;
import com.mogujie.tt.config.IntentConstant;
import com.mogujie.tt.config.UrlConstant;
import com.mogujie.tt.utils.IMUIHelper;
import com.mogujie.tt.imservice.event.LoginEvent;
import com.mogujie.tt.imservice.event.SocketEvent;
import com.mogujie.tt.imservice.manager.IMLoginManager;
import com.mogujie.tt.imservice.service.IMService;
import com.mogujie.tt.ui.base.TTBaseActivity;
import com.mogujie.tt.imservice.support.IMServiceConnector;
import com.mogujie.tt.utils.Logger;

import de.greenrobot.event.EventBus;


/**
 * @YM 1. 链接成功之后，直接判断是否loginSp是否可以直接登陆
 * true: 1.可以登陆，从DB中获取历史的状态
 * 2.建立长连接，请求最新的数据状态 【网络断开没有这个状态】
 * 3.完成
 * <p/>
 * false:1. 不能直接登陆，跳转到登陆页面
 * 2. 请求消息服务器地址，链接，验证，触发loginSuccess
 * 3. 保存登陆状态
 */
public class LoginActivity extends TTBaseActivity {

    private Logger logger = Logger.getLogger(LoginActivity.class);
    private Handler uiHandler = new Handler();
    private EditText mNameView;
    private EditText mPasswordView;
    private View loginPage;
    private View splashPage;
    private View mLoginStatusView;
    private TextView mSwitchLoginServer;
    private InputMethodManager intputManager;

    private IMService imService;
    private boolean autoLogin = true;
    private boolean loginSuccess = false;

    private IMServiceConnector imServiceConnector = new IMServiceConnector() {
        @Override
        public void onServiceDisconnected() {
        }

        @Override
        public void onIMServiceConnected() {
            logger.d("login#onIMServiceConnected");
            imService = imServiceConnector.getIMService();
            try {
                do {
                    if (imService == null) {
                        //后台服务启动链接失败
                        break;
                    }
                    IMLoginManager loginManager = imService.getLoginManager();
                    LoginSp loginSp = imService.getLoginSp();
                    if (loginManager == null || loginSp == null) {
                        // 无法获取登陆控制器
                        break;
                    }

                    LoginSp.SpLoginIdentity loginIdentity = loginSp.getLoginIdentity();
                    if (loginIdentity == null) {
                        // 之前没有保存任何登陆相关的，跳转到登陆页面
                        break;
                    }

                    mNameView.setText(loginIdentity.getLoginName());
                    if (TextUtils.isEmpty(loginIdentity.getPwd())) {
                        // 密码为空，可能是loginOut
                        break;
                    }
                    mPasswordView.setText(loginIdentity.getPwd());

                    if (autoLogin == false) {
                        break;
                    }

                    handleGotLoginIdentity(loginIdentity);
                    return;
                } while (false);

                // 异常分支都会执行这个
                handleNoLoginIdentity();
            } catch (Exception e) {
                // 任何未知的异常
                logger.w("loadIdentity failed");
                handleNoLoginIdentity();
            }
        }
    };

    /**
     * 跳转到登陆的页面
     */
    private void handleNoLoginIdentity() {
        logger.i("login#handleNoLoginIdentity");
        uiHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                showLoginPage();
            }
        }, 1000);
    }

    /**
     * 自动登陆
     */
    private void handleGotLoginIdentity(final LoginSp.SpLoginIdentity loginIdentity) {
        logger.i("login#handleGotLoginIdentity");

        uiHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                logger.d("login#start auto login");
                if (imService == null || imService.getLoginManager() == null) {
                    Toast.makeText(LoginActivity.this, getString(R.string.login_failed), Toast.LENGTH_SHORT).show();
                    showLoginPage();
                }
                imService.getLoginManager().login(loginIdentity);
            }
        }, 500);
    }


    private void showLoginPage() {
        splashPage.setVisibility(View.GONE);
        loginPage.setVisibility(View.VISIBLE);
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        intputManager = (InputMethodManager) getSystemService(this.INPUT_METHOD_SERVICE);
        logger.d("login#onCreate");

        SystemConfigSp.instance().init(getApplicationContext());
        if (TextUtils.isEmpty(SystemConfigSp.instance().getStrConfig(SystemConfigSp.SysCfgDimension.LOGINSERVER))) {
            SystemConfigSp.instance().setStrConfig(SystemConfigSp.SysCfgDimension.LOGINSERVER, UrlConstant.ACCESS_MSG_ADDRESS);
        }

        imServiceConnector.connect(LoginActivity.this);
        EventBus.getDefault().register(this);

        setContentView(R.layout.tt_activity_login);
        mSwitchLoginServer = (TextView)findViewById(R.id.sign_switch_login_server);
        mSwitchLoginServer.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View view) {
                AlertDialog.Builder builder = new AlertDialog.Builder(new ContextThemeWrapper(LoginActivity.this, android.R.style.Theme_Holo_Light_Dialog));
                LayoutInflater inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                View dialog_view = inflater.inflate(R.layout.tt_custom_dialog, null);
                final EditText editText = (EditText)dialog_view.findViewById(R.id.dialog_edit_content);
                editText.setText(SystemConfigSp.instance().getStrConfig(SystemConfigSp.SysCfgDimension.LOGINSERVER));
                TextView textText = (TextView)dialog_view.findViewById(R.id.dialog_title);
                textText.setText(R.string.switch_login_server_title);
                builder.setView(dialog_view);
                builder.setPositiveButton(getString(R.string.tt_ok), new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {

                        if(!TextUtils.isEmpty(editText.getText().toString().trim()))
                        {
                            SystemConfigSp.instance().setStrConfig(SystemConfigSp.SysCfgDimension.LOGINSERVER,editText.getText().toString().trim());
                            dialog.dismiss();
                        }
                    }
                });
                builder.setNegativeButton(getString(R.string.tt_cancel), new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                    }
                });
                builder.show();
            }
        });

        mNameView = (EditText) findViewById(R.id.name);
        mPasswordView = (EditText) findViewById(R.id.password);
        mPasswordView.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int id, KeyEvent keyEvent) {

                if (id == R.id.login || id == EditorInfo.IME_NULL) {
                    attemptLogin();
                    return true;
                }
                return false;
            }
        });
        mLoginStatusView = findViewById(R.id.login_status);
        findViewById(R.id.sign_in_button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                logger.d("sign_in_button onClick");
                intputManager.hideSoftInputFromWindow(mPasswordView.getWindowToken(), 0);
                attemptLogin();
            }
        });
        initAutoLogin();
    }

    private void initAutoLogin() {
        logger.i("login#initAutoLogin");

        splashPage = findViewById(R.id.splash_page);
        loginPage = findViewById(R.id.login_page);
        autoLogin = shouldAutoLogin();

        splashPage.setVisibility(autoLogin ? View.VISIBLE : View.GONE);
        loginPage.setVisibility(autoLogin ? View.GONE : View.VISIBLE);

        loginPage.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {

                if (mPasswordView != null) {
                    intputManager.hideSoftInputFromWindow(mPasswordView.getWindowToken(), 0);
                }

                if (mNameView != null) {
                    intputManager.hideSoftInputFromWindow(mNameView.getWindowToken(), 0);
                }

                return false;
            }
        });

        if (autoLogin) {
            Animation splashAnimation = AnimationUtils.loadAnimation(this, R.anim.login_splash);
            if (splashAnimation == null) {
                logger.e("login#loadAnimation login_splash failed");
                return;
            }

            splashPage.startAnimation(splashAnimation);
        }
    }

    // 主动退出的时候， 这个地方会有值,更具pwd来判断
    private boolean shouldAutoLogin() {
        Intent intent = getIntent();
        if (intent != null) {
            boolean notAutoLogin = intent.getBooleanExtra(IntentConstant.KEY_LOGIN_NOT_AUTO, false);
            logger.d("login#notAutoLogin:%s", notAutoLogin);
            if (notAutoLogin) {
                return false;
            }
        }
        return true;
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();

        imServiceConnector.disconnect(LoginActivity.this);
        EventBus.getDefault().unregister(this);
        splashPage = null;
        loginPage = null;
    }


    public void attemptLogin() {
        String loginName = mNameView.getText().toString();
        String mPassword = mPasswordView.getText().toString();
        boolean cancel = false;
        View focusView = null;

        if (TextUtils.isEmpty(mPassword)) {
            Toast.makeText(this, getString(R.string.error_pwd_required), Toast.LENGTH_SHORT).show();
            focusView = mPasswordView;
            cancel = true;
        }

        if (TextUtils.isEmpty(loginName)) {
            Toast.makeText(this, getString(R.string.error_name_required), Toast.LENGTH_SHORT).show();
            focusView = mNameView;
            cancel = true;
        }

        if (cancel) {
            focusView.requestFocus();
        } else {
            showProgress(true);
            if (imService != null) {
//				boolean userNameChanged = true;
//				boolean pwdChanged = true;
                loginName = loginName.trim();
                mPassword = mPassword.trim();
//                logger.d("imService.getLoginManager().login(loginName, mPassword)");
                imService.getLoginManager().login(loginName, mPassword);
            }
        }
    }

    private void showProgress(final boolean show) {
        if (show) {
            mLoginStatusView.setVisibility(View.VISIBLE);
        } else {
            mLoginStatusView.setVisibility(View.GONE);
        }
    }

    // 为什么会有两个这个
    // 可能是 兼容性的问题 导致两种方法onBackPressed
    @Override
    public void onBackPressed() {
        logger.d("login#onBackPressed");
        //imLoginMgr.cancel();
        // TODO Auto-generated method stub
        super.onBackPressed();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
//        if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0) {
//            LoginActivity.this.finish();
//            return true;
//        }
        return super.onKeyDown(keyCode, event);
    }


    @Override
    protected void onStop() {
        super.onStop();
    }

    /**
     * ----------------------------event 事件驱动----------------------------
     */
//    http://blog.csdn.net/harvic880925/article/details/40787203
//    1、onEvent
//    2、onEventMainThread
//    3、onEventBackgroundThread
//    4、onEventAsync
//    onEvent:如果使用onEvent作为订阅函数，那么该事件在哪个线程发布出来的，onEvent就
// 会在这个线程中运行，也就是说发布事件和接收事件线程在同一个线程。使用这个方法时，在onEvent
// 方法中不能执行耗时操作，如果执行耗时操作容易导致事件分发延迟。
//    onEventMainThread:如果使用onEventMainThread作为订阅函数，那么不论事件是在哪
// 个线程中发布出来的，onEventMainThread都会在UI线程中执行，接收事件就会在UI线程中运行，
// 这个在Android中是非常有用的，因为在Android中只能在UI线程中跟新UI，所以在onEvnetMainThread
// 方法中是不能执行耗时操作的。
//    onEventBackground:如果使用onEventBackgrond作为订阅函数，那么如果事件是在UI线
// 程中发布出来的，那么onEventBackground就会在子线程中运行，如果事件本来就是子线程中发布
// 出来的，那么onEventBackground函数直接在该子线程中执行。
//    onEventAsync：使用这个函数作为订阅函数，那么无论事件在哪个线程发布，都会创建新的子
// 线程在执行onEventAsync.
    public void onEventMainThread(LoginEvent event) {
        switch (event) {
            case LOCAL_LOGIN_SUCCESS:
            case LOGIN_OK:
                onLoginSuccess();
                break;
            case LOGIN_AUTH_FAILED:
            case LOGIN_INNER_FAILED:
                if (!loginSuccess)
                    onLoginFailure(event);
                break;
        }
    }


    public void onEventMainThread(SocketEvent event) {
        switch (event) {
            case CONNECT_MSG_SERVER_FAILED:
            case REQ_MSG_SERVER_ADDRS_FAILED:
                if (!loginSuccess)
                    onSocketFailure(event);
                break;
        }
    }

    private void onLoginSuccess() {
        logger.i("login#onLoginSuccess");
        loginSuccess = true;
        Intent intent = new Intent(LoginActivity.this, MainActivity.class);
        startActivity(intent);
        LoginActivity.this.finish();
    }

    private void onLoginFailure(LoginEvent event) {
        logger.e("login#onLoginError -> errorCode:%s", event.name());
        showLoginPage();
        String errorTip = getString(IMUIHelper.getLoginErrorTip(event));
        logger.d("login#errorTip:%s", errorTip);
        mLoginStatusView.setVisibility(View.GONE);
        Toast.makeText(this, errorTip, Toast.LENGTH_SHORT).show();
    }

    private void onSocketFailure(SocketEvent event) {
        logger.e("login#onLoginError -> errorCode:%s,", event.name());
        showLoginPage();
        String errorTip = getString(IMUIHelper.getSocketErrorTip(event));
        logger.d("login#errorTip:%s", errorTip);
        mLoginStatusView.setVisibility(View.GONE);
        Toast.makeText(this, errorTip, Toast.LENGTH_SHORT).show();
    }
}
