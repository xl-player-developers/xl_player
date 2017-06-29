package com.cls.xl.xl;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.xl.media.library.base.OnErrorCodeListener;
import com.xl.media.library.base.OnPlayerStatusChangeListener;
import com.xl.media.library.base.XLPlayer;

import static java.lang.String.format;

public class SinglePlayerActivity extends Activity implements View.OnClickListener {
    private SurfaceView mGLSurfaceView;
    private XLPlayer xlPlayer;
    SeekBar videoSeekBar;
    int videoProNow;
    TextView playIcon;
    TextView rateView;
    GestureDetector mGestureDetector;
    ScaleGestureDetector mScaleGestureDetector;
    float nowScale = 1f;//缩放的取值范围,0.5f 到 2f
    String url;
    boolean isBackPlay = false;
    @SuppressLint("HandlerLeak")
    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case 1:
                    videoProNow = (int) xlPlayer.getVideoCurrentTime();
                    ((TextView) findViewById(R.id.currentTime)).setText(formatTime(videoProNow));
                    videoSeekBar.setProgress(videoProNow);
                    handler.sendEmptyMessageDelayed(1, 1000);
                    XLPlayer.Statistics st = xlPlayer.getStatistics();
                    System.out.println("fps = " + st.getFps() + " , bps = " + st.getFormatBps() + ", buffer = " + st.getBufferLength() + "ms");
                    break;
            }
        }
    };

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_single);
        url = getIntent().getStringExtra("media_url");
        mGestureDetector = new GestureDetector(this, simpleOnGestureListener);
        mScaleGestureDetector = new ScaleGestureDetector(this, scaleGestureListener);
        mGLSurfaceView = (SurfaceView) findViewById(R.id.m_xlsurface);
        mGLSurfaceView.setOnTouchListener(playerViewTouchListener);
        mGLSurfaceView.setKeepScreenOn(true);
        mGLSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                xlPlayer.setSurface(mGLSurfaceView.getHolder().getSurface());
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                System.out.println("surfaceDestroyed");
            }
        });
        videoSeekBar = (SeekBar) findViewById(R.id.video_progress);
        videoSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                handler.removeMessages(1);
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                xlPlayer.seekTo(seekBar.getProgress());
                handler.sendEmptyMessage(1);
            }
        });
        playIcon = (TextView) findViewById(R.id.start);
        playIcon.setOnClickListener(playListener);
        rateView = (TextView) findViewById(R.id.now_rate);
        findViewById(R.id.seek_front).setOnClickListener(this);
        findViewById(R.id.seek_back).setOnClickListener(this);
        findViewById(R.id.stop).setOnClickListener(this);
        findViewById(R.id.clockwise).setOnClickListener(this);
        findViewById(R.id.anti_clockwise).setOnClickListener(this);
        findViewById(R.id.rate_plus).setOnClickListener(this);
        findViewById(R.id.rate_minus).setOnClickListener(this);
        findViewById(R.id.setting).setOnClickListener(this);
        findViewById(R.id.player_nomal).setOnClickListener(this);
        findViewById(R.id.vr_model_ball).setOnClickListener(this);
        findViewById(R.id.vr_model_building).setOnClickListener(this);
        findViewById(R.id.vr_model_little_star).setOnClickListener(this);
        findViewById(R.id.vr_model_two_eye).setOnClickListener(this);
        findViewById(R.id.vr_model_pic).setOnClickListener(this);
        findViewById(R.id.enable_tracker).setOnClickListener(this);
        ((CheckBox) findViewById(R.id.back_play)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                isBackPlay = isChecked;
            }
        });
        xlPlayer = new XLPlayer(this);
        xlPlayer.setForceSwDecode(MainActivity.forceSwDecode);
        xlPlayer.setOnPlayerStatusChangeListener(playerStatusChangeListener);
        xlPlayer.setOnErrorCodeListener(new OnErrorCodeListener() {
            @Override
            public void onGetErrorCode(int errorCode) {
                switch (errorCode) {
                    case 100:
                        Toast.makeText(SinglePlayerActivity.this, "cound not open url", Toast.LENGTH_SHORT).show();
                        break;
                    default:
                        Toast.makeText(SinglePlayerActivity.this, "has error ,code is " + errorCode, Toast.LENGTH_SHORT).show();
                        break;
                }
            }
        });
        xlPlayer.playVideo(url);
    }

    View.OnTouchListener playerViewTouchListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            mScaleGestureDetector.onTouchEvent(event);
            mGestureDetector.onTouchEvent(event);
            return true;
        }
    };

    ScaleGestureDetector.SimpleOnScaleGestureListener scaleGestureListener = new ScaleGestureDetector.SimpleOnScaleGestureListener() {
        @Override
        public boolean onScale(ScaleGestureDetector detector) {
            nowScale *= detector.getScaleFactor();
            nowScale = nowScale > 2f ? 2f : (nowScale < 0.5f ? 0.5f : nowScale);
            xlPlayer.setScale(nowScale);
            return true;
        }
    };
    float startx, starty;
    GestureDetector.SimpleOnGestureListener simpleOnGestureListener = new GestureDetector.SimpleOnGestureListener() {
        @Override
        public boolean onDown(MotionEvent e) {
            startx = e.getX();
            starty = e.getY();
            return super.onDown(e);
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
            if (xlPlayer.getModelType() == XLPlayer.MODEL_TYPE.Planet) {
                float centerX = mGLSurfaceView.getWidth() / 2f;
                float centerY = mGLSurfaceView.getHeight() / 2f;
                float startAngle = (float) Math.atan2(starty - centerY, startx - centerX);
                float endAngle = (float) Math.atan2(e2.getY() - centerY, e2.getX() - centerX);
                xlPlayer.setRotation(0, 0, startAngle - endAngle);
                startx = e2.getX();
                starty = e2.getY();
            } else {
                xlPlayer.setRotation((float) (distanceY / 10f / 180f * Math.PI), (float) (distanceX / 10f / 180f * Math.PI), 0);
            }

            return true;
        }
    };


    View.OnClickListener playListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            xlPlayer.playVideo(url, 0, XLPlayer.MODEL_TYPE.Rect);
        }
    };

    OnPlayerStatusChangeListener playerStatusChangeListener = new OnPlayerStatusChangeListener() {
        @Override
        public void onStart() {
            int videoProMax = (int) xlPlayer.getVideoTotalTime();
            ((TextView) findViewById(R.id.duration)).setText(formatTime(videoProMax));
            videoSeekBar.setMax(videoProMax);
            handler.sendEmptyMessageDelayed(1, 1000);
            playIcon.setBackground(getResources().getDrawable(R.drawable.pause, null));
            playIcon.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    xlPlayer.pauseVideo();
                }
            });
        }

        @Override
        public void onPause() {
            super.onPause();
            handler.removeCallbacksAndMessages(null);
            playIcon.setBackground(getResources().getDrawable(R.drawable.play, null));
            handler.sendEmptyMessageDelayed(1, 1000);
            playIcon.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    xlPlayer.resumeVideo();
                }
            });
        }

        @Override
        public void onResume() {
            super.onResume();
            handler.sendEmptyMessageDelayed(1, 1000);
            playIcon.setBackground(getResources().getDrawable(R.drawable.pause, null));
            playIcon.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    xlPlayer.pauseVideo();
                }
            });
        }

        @Override
        public void onStop() {
            super.onStop();
            if (handler != null) {
                videoSeekBar.setProgress(0);
                ((TextView) findViewById(R.id.currentTime)).setText(formatTime(0));
                ((TextView) findViewById(R.id.duration)).setText(formatTime(0));
                handler.removeCallbacksAndMessages(null);
                playIcon.setBackground(getResources().getDrawable(R.drawable.play, null));
                playIcon.setOnClickListener(playListener);
            }
        }

        @Override
        public void onPrepared() {
            super.onPrepared();
            playIcon.setOnClickListener(playListener);
        }
    };


    @Override
    protected void onResume() {
        super.onResume();
        if (isBackPlay) {
            xlPlayer.setPlayBackground(false);
        } else {
            xlPlayer.onResume();
        }

    }

    @Override
    protected void onPause() {
        super.onPause();
        if (isBackPlay) {
            xlPlayer.setPlayBackground(true);
        } else {
            xlPlayer.onPause();
        }

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        handler.removeCallbacksAndMessages(null);
        handler = null;
        xlPlayer.releaseVideo();
        mGLSurfaceView = null;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    String formatTime(int time) {
        int h = time / 60 / 60;
        int m = (time - h * 3600) / 60;
        int s = time - h * 3600 - m * 60;
        return h + ":" + m + ":" + s;
    }

    @Override
    public void onClick(View v) {
        float nowRate;
        switch (v.getId()) {
            case R.id.seek_front:
                xlPlayer.seekTime(10);
                break;
            case R.id.seek_back:
                xlPlayer.seekTime(-10);
                break;
            case R.id.stop:
                xlPlayer.stopVideo();
                break;
            case R.id.clockwise:
                xlPlayer.rotate(false);
                break;
            case R.id.anti_clockwise:
                xlPlayer.rotate(true);
                break;
            case R.id.rate_plus:
                nowRate = Float.parseFloat(rateView.getText().toString());
                if (nowRate < 2.0) {
                    nowRate += 0.1f;
                    xlPlayer.setRate(nowRate);
                    rateView.setText(format("%.1f", nowRate));
                }
                break;
            case R.id.rate_minus:
                nowRate = Float.parseFloat(rateView.getText().toString());
                if (nowRate > 0.5) {
                    nowRate -= 0.1f;
                    xlPlayer.setRate(nowRate);
                    rateView.setText(format("%.1f", nowRate));
                }
                break;
            case R.id.setting:
                findViewById(R.id.choose_vr_mode_layout).setVisibility(findViewById(R.id.choose_vr_mode_layout).getVisibility() == View.INVISIBLE ? View.VISIBLE : View.INVISIBLE);
                break;
            case R.id.player_nomal:
                nowScale = 1f;
                xlPlayer.setEnableTracker(false);
                findViewById(R.id.choose_vr_mode_layout).setVisibility(View.INVISIBLE);
                xlPlayer.changeModel(XLPlayer.MODEL_TYPE.Rect);
                break;
            case R.id.vr_model_ball:
                nowScale = 1f;
                xlPlayer.setEnableTracker(true);
                findViewById(R.id.choose_vr_mode_layout).setVisibility(View.INVISIBLE);
                xlPlayer.changeModel(XLPlayer.MODEL_TYPE.Ball);
                break;
            case R.id.vr_model_building:
                nowScale = 1f;
                xlPlayer.setEnableTracker(false);
                findViewById(R.id.choose_vr_mode_layout).setVisibility(View.INVISIBLE);
                xlPlayer.changeModel(XLPlayer.MODEL_TYPE.Architecture);
                break;
            case R.id.vr_model_little_star:
                nowScale = 1f;
                xlPlayer.setEnableTracker(false);
                findViewById(R.id.choose_vr_mode_layout).setVisibility(View.INVISIBLE);
                xlPlayer.changeModel(XLPlayer.MODEL_TYPE.Planet);
                break;
            case R.id.vr_model_pic:
                nowScale = 1f;
                xlPlayer.setEnableTracker(false);
                findViewById(R.id.choose_vr_mode_layout).setVisibility(View.INVISIBLE);
                xlPlayer.changeModel(XLPlayer.MODEL_TYPE.Expand);
                break;
            case R.id.vr_model_two_eye:
                nowScale = 1f;
                xlPlayer.setEnableTracker(true);
                findViewById(R.id.choose_vr_mode_layout).setVisibility(View.INVISIBLE);
                xlPlayer.changeModel(XLPlayer.MODEL_TYPE.VR);
                break;
            case R.id.enable_tracker:
                xlPlayer.setEnableTracker(!xlPlayer.getEnableTracker());
                break;
        }
    }

}
