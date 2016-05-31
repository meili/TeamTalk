package com.mogujie.tt.ui.activity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import com.mogujie.tt.R;
import com.mogujie.tt.ui.base.TTBaseFragmentActivity;

public class ConfirmAudioActivity extends TTBaseFragmentActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.tt_activity_confirm_audio);

        AddbtnOK();
        AddbtnCancel();
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();
    }

    /**
     *
     */
    private void AddbtnOK(){
        Button btnok = (Button)findViewById(R.id.btnOK);
        btnok.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //
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
