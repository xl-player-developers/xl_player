# XLPlayer
[ ![last version](https://api.bintray.com/packages/xl-player-developers/xl-player-library/xl-player-arm64v8a/images/download.svg) ](#)

## 功能介绍
* XLPlayer一般情况下使用MediaCodec->SurfaceTexture->OpenGL ES工作模式,性能优异,小米3也可以轻松播放4K VR视频.
* 支持音/视频播放,暂停,停止,seek,获取总时长,已播时长等播放器常用功能
* 支持几乎所有的媒体封装格式,包括但不限于.mp4 .mkv .flv rtmp hls .webm .mov等.
* H263/H264/H265/MPEG4/VP8/VP9 支持硬件加速(部分设备不支持H265 VP9).
* 支持音频自适应最佳采样率播放
* 支持音视频变速(0.5 - 2.0)不变调播放
* 支持VR视频播放,VR视频模式动态切换(球模式、盒子模式、小行星模式、 建筑学模式、展开模式)
* 球模式、盒子模式、建筑学模式支持陀螺仪控制.
* 盒子模式带有透镜畸变和色散的补偿
* 支持播放时动态切换surface
* 支持动态切换画面方向
* 支持后台播放
* 支持获取已缓存时长,当前帧率,当前下载速度
* 支持视频本身自带rotation的旋转
* 支持强制软解(默认自适应)
* 支持列表播放
* 支持设置缓存时长和大小

## [DemoApk下载](simple_apk/app-debug.apk)

## 使用说明

我们的播放器库支持minSdkVersion 16+, 不过我们建议使用minSdkVersion >= 21.
原因是当minSdkVersion >= 21 时, 硬件解码器会使用Android Native层接口: AMediaCodec,
否则需要通过反射使用java层接口Mediaodec,效率略低.

### 直接在module下的build.gradle添加

    compile 'com.xl.media.library:xl-player-java:<LAST-VERSION>'//必须

    //适配minsdkversion >= 16
    compile 'com.xl.media.library:xl-player-armeabi:<LAST-VERSION>'
    compile 'com.xl.media.library:xl-player-armv7a:<LAST-VERSION>'
    compile 'com.xl.media.library:xl-player-arm64v8a:<LAST-VERSION>'

    //适配minsdkversion >= 21 的版本,与>=16版本冲突
    //compile 'com.xl.media.library:xl-player-armeabi-21:<LAST-VERSION>'
    //compile 'com.xl.media.library:xl-player-armv7a-21:<LAST-VERSION>'
    //compile 'com.xl.media.library:xl-player-arm64v8a-21:<LAST-VERSION>'

默认情况下,建议三个abi的库版本都添加,以提高app性能.如果为了app包大小考虑,可只添加

    compile 'com.xl.media.library:xl-player-java:<LAST-VERSION>'
    compile 'com.xl.media.library:xl-player-armeabi:<LAST-VERSION>'

有时候第三方的sdk并没有提供armeabi版本的sdk,只提供了armv7a的sdk,则需要替换为

    compile 'com.xl.media.library:xl-player-java:<LAST-VERSION>'
    compile 'com.xl.media.library:xl-player-armv7a:<LAST-VERSION>'

以防止app报找不到第三方sdk.so的错误.

    
### 创建一个简单的播放器

注意：surface一定要在成功创建之后再传给player,一般做法是在`SurfaceHolder`的Callback:`surfaceCreated`函数里面调用`xlPlayer.setSurface`.

java code [`SimpleDemoActivity.java`](app/src/main/java/com/cls/xl/xl/SimpleDemoActivity.java)

Android Layout [`activity_simple_demo.xml`](app/src/main/res/layout/activity_whack_a_mole.xml)


## [api说明](https://github.com/xl-player-developers/xl_player/wiki)
