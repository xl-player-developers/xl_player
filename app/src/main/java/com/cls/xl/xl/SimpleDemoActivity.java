package com.cls.xl.xl;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.xl.media.library.base.XLPlayer;

public class SimpleDemoActivity extends Activity {
    private XLPlayer xlPlayer;
    private SurfaceView surfaceView;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_simple_demo);
        xlPlayer = new XLPlayer(this);
        surfaceView = ((SurfaceView) findViewById(R.id.m_xlsurface));
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                xlPlayer.setSurface(holder.getSurface());
            }
            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                xlPlayer.resize(width, height);
            }
            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                xlPlayer.releasePlayer();
            }
        });
        xlPlayer.playVideo("http://storage.googleapis.com/exoplayer-test-media-1/mkv/android-screens-lavf-56.36.100-aac-avc-main-1280x720.mkv");
    }
}
