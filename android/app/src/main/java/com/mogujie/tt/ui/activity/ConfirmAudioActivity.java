package com.mogujie.tt.ui.activity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.mogujie.tt.DB.entity.UserEntity;
import com.mogujie.tt.R;
import com.mogujie.tt.config.IntentConstant;
import com.mogujie.tt.imservice.service.IMService;
import com.mogujie.tt.imservice.support.IMServiceConnector;
import com.mogujie.tt.ui.base.TTBaseFragmentActivity;

public class ConfirmAudioActivity extends TTBaseFragmentActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.tt_activity_confirm_audio);

        AddbtnOK();
        AddbtnCancel();

        imServiceConnector.connect(this);
        currentSessionKey = getIntent().getStringExtra(IntentConstant.KEY_SESSION_KEY);

    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();

        imServiceConnector.disconnect(this);
    }
    private String currentSessionKey;

    private UserEntity loginUser;
    private IMService imService;
  //  private PeerEntity peerEntity;

    private IMServiceConnector imServiceConnector =new IMServiceConnector(){
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
    // 触发条件,imservice链接成功，或者newIntent
    private void initData() {

        loginUser = imService.getLoginManager().getLoginInfo();
//        peerEntity = imService.getSessionManager().findPeerEntity(currentSessionKey);

    }

    /**
     *
     */
    private void AddbtnOK(){
        Button btnok = (Button)findViewById(R.id.btnOK);
        btnok.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                TextMessage textMessage = TextMessage.buildForSend(1, loginUser, peerEntity);
//
//                //
//                SocketAddress serverAddress = new InetSocketAddress("123.57.71.215", 8132);
//                // 发送指令给udp_server
//
//                imService.getNatServerMgr().sendUDPMessage(textMessage,serverAddress);
            }
        });
    }

    /**
     *
     */
    private void AddbtnCancel(){
        Button btnCancel = (Button) findViewById(R.id.btnCancel);
        btnCancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

            }
        });
    }
}
