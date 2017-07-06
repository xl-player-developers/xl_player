# XLPlayer
高性能android播放器,支持播放vr视频  
A high performance Android media player, base on ffmpeg and MediaCodec, support VR video.
## 功能介绍 Introduction
XLPlayer支持几乎所有的媒体封装格式,包括但不限于mp4 mkv flv rtmp hls webm mov等.  
当视频编码方式为H263 H264 H265 MPEG4 VP8 VP9时,XLPlayer会使用MediaCodec硬件加速解码(部分设备不支持H265 VP9).  
音频全部使用libavcodec做为解码器,并且使用libavfilter进行重采样以适应不同手机的最佳音频采样率.  

XLPlayer支持以几种模型渲染VR视频,球模式、盒子模式(带有透镜畸变和色散的补偿)、小行星模式、  
建筑学模式(直线不会变弯,其他与球模式相同)、展开模式,其中球模式、盒子模式、建筑学模式支持陀螺仪控制.

XLPlayer还支持变速播放,0.5-2.0倍速任意调整.

XLPlayer一般情况下使用MediaCodec->SurfaceTexture->OpenGL ES工作模式,性能优异,小米3也可以轻松播放4K VR视频.



## 使用说明 Usage

### step1: 把库加入到您的项目中 add library to your project
我们的播放器库支持minSdkVersion 16+, 不过我们建议使用minSdkVersion 21+.  
原因是当minSdkVersion >= 21 时, 硬件解码器会使用Android Native层接口: AMediaCodec,   
否则需要通过反射使用java层接口Mediaodec,效率略低.  
Our player library support minSdkVersion >= 16, but we suggest you use minSdkVersion >= 21.  
Because when minSdkVersion >= 21, Hardware acceleration will use Native interface `AMediaCodec`,   
otherwise it will **reflect** java interface `MediaCodec`, Efficiency is slightly lower.  

在*build.gradle*文件中加入您需要的Native库(xl-player-armeabi/armv7a/arm64v8a),以及java接口库(xl-player-java)  
Add Native library which you need(xl-player-armeabi/armv7a/arm64v8a),and the java interface library(xl-player-java) to _build.gradle_.
##### minSdkVersion 21
build.gradle

    ...
    dependencies {
        ...
        compile project(':xl-player-armeabi-21')
        compile project(':xl-player-armv7a-21')
        compile project(':xl-player-arm64v8a-21')
        
        compile project(':xl-player-java')
    }
    
##### minSdkVersion 16
build.gradle

    ...
    dependencies {
        ...
        compile project(':xl-player-armeabi')
        compile project(':xl-player-armv7a')
        compile project(':xl-player-arm64v8a')
        
        compile project(':xl-player-java')
    }
    
### step2: 创建一个播放器 create a player
创建XLPlayer实例需要一个`android.content.Context`,一般使用`Activity`.  
播放视频,还需要给播放器指定一个`Surface`,然后就可以调用播放了^_^  
注意：surface一定要在成功创建之后再传给player,一般做法是在`SurfaceHolder`的Callback:`surfaceCreated`函数里面调用`xlPlayer.setSurface`.  
Create XLPlayer instance need an `android.content.Context`, as usual we pass an `Activity` to the constructor.  
We need a surface set to player,then we can play ^_^  
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