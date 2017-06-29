package com.cls.xl.xl;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.xl.media.library.base.XLPlayer;

import java.util.ArrayList;
import java.util.List;


public class Whack_a_mole_Activity extends Activity implements View.OnClickListener {
    private XLPlayer xlPlayer;
    private List<View> viewList = new ArrayList<>(4);
    private int currentIndex = 0;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_whack_a_mole);
        xlPlayer = new XLPlayer(this);
        ((SurfaceView) findViewById(R.id.surface_1)).getHolder().addCallback(new BaseHolderCallback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                xlPlayer.setSurface(holder.getSurface());
            }
        });
        viewList.add(findViewById(R.id.surface_1));
        viewList.add(findViewById(R.id.surface_2));
        viewList.add(findViewById(R.id.surface_3));
        viewList.add(findViewById(R.id.surface_4));
        for(View v : viewList){
            v.setOnClickListener(this);
        }
        xlPlayer.playVideo("http://storage.googleapis.com/exoplayer-test-media-1/mkv/android-screens-lavf-56.36.100-aac-avc-main-1280x720.mkv");
    }

    @Override
    public void onClick(View v) {
        if(viewList.get(currentIndex) == v){
            int i = (int)(Math.random() * 3.9999);
            xlPlayer.setSurface(((SurfaceView) viewList.get(i)).getHolder().getSurface());
            currentIndex = i;
        }

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        xlPlayer.releaseVideo();
    }
}
