# XLPlayer
A high performance Android media player, base on ffmpeg and MediaCodec, support VR video.

 [中文](README.md) | [FFmpeg](http://ffmpeg.org/)

## Introduction

XLPlayer support almost all of media formats. Includ .mp4 .mkv .flv rtmp hls .webm .mov and so on.
When video coded by H263/H264/H265/MPEG4/VP8/VP9, XLPlayer will use MediaCodec decode video stream(some device not support H265 VP9).
Audio stream will use libavcodec decode, and use libavfilter resample to adapt to different device's best audio sample rate.

XLPlayer support some model types to rend VR video, like ball_model / google_card_board / little_plant / expand_model.
ball_model and google_card_board support gyroscope control.

XLPlayer support Variable Speed Playback, you can set playback with 0.5x-2.0x speed.

As usual XLPlayer work in MediaCodec->SurfaceTexture->OpenGL ES mode, this mode has high performance.

##  Usage

### step1:add library to your project
Our player library support minSdkVersion >= 16, but we suggest you use minSdkVersion >= 21.
Because when minSdkVersion >= 21, Hardware acceleration will use Native interface `AMediaCodec`,
otherwise it will **reflect** java interface `MediaCodec`, Efficiency is slightly lower.

Add Native library which you need(xl-player-armeabi/armv7a/arm64v8a),and the java interface library(xl-player-java) to _build.gradle_.
##### minSdkVersion 21
build.gradle

	compile 'com.xl.media.library:xl-player-java:0.0.5'
    compile 'com.xl.media.library:xl-player-armeabi-21:0.0.5'
    compile 'com.xl.media.library:xl-player-armv7a-21:0.0.5'
    compile 'com.xl.media.library:xl-player-arm64v8a-21:0.0.5'

##### minSdkVersion 16
build.gradle

    compile 'com.xl.media.library:xl-player-java:0.0.5'
    compile 'com.xl.media.library:xl-player-armeabi:0.0.5'
    compile 'com.xl.media.library:xl-player-armv7a:0.0.5'
    compile 'com.xl.media.library:xl-player-arm64v8a:0.0.5'

### step2:  create a simple player
Create XLPlayer instance need an `android.content.Context`
We need a surface set to player,then we can play
note: surface mast set to player after it created. as usual we call `xlPlayer.setSurface` in `SurfaceHolder`'s Callback:`surfaceCreated` function.

java code [`SimpleDemoActivity.java`](app/src/main/java/com/cls/xl/xl/SimpleDemoActivity.java)

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
                    xlPlayer.releaseVideo();
                }
            });
            xlPlayer.playVideo("http://storage.googleapis.com/exoplayer-test-media-1/mkv/android-screens-lavf-56.36.100-aac-avc-main-1280x720.mkv");
        }
    }

Android Layout [`activity_simple_demo.xml`](app/src/main/res/layout/activity_whack_a_mole.xml)

    <?xml version="1.0" encoding="utf-8"?>
    <RelativeLayout xmlns:android="http://schemas.android.com/apk/res/android"
        xmlns:tools="http://schemas.android.com/tools"
        android:layout_width="match_parent"
        android:layout_height="match_parent"
        android:background="#000000">
        <SurfaceView
            android:id="@+id/m_xlsurface"
            android:layout_width="match_parent"
            android:layout_height="match_parent"
            android:layout_alignParentStart="true"
            android:layout_alignParentTop="true"/>
    </RelativeLayout>
