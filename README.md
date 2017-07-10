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

## 使用说明

如果直接clone and run项目的话,建议先修改gradle/wrapper/gradle-wrapper.properties的 distributionUrl 和 project的build.gradle的classpath,否则可能会需要升级AndroidStudio的版本才能运行项目.

我们的播放器库支持minSdkVersion 16+, 不过我们建议使用minSdkVersion >= 21.
原因是当minSdkVersion >= 21 时, 硬件解码器会使用Android Native层接口: AMediaCodec,
否则需要通过反射使用java层接口Mediaodec,效率略低.

### 在module下的build.gradle添加

    compile 'com.xl.media.library:xl-player-java:<LAST-VERSION>'//必须

    //适配minsdkversion >= 16
    compile 'com.xl.media.library:xl-player-armeabi:<LAST-VERSION>'
    compile 'com.xl.media.library:xl-player-armv7a:<LAST-VERSION>'
    compile 'com.xl.media.library:xl-player-arm64v8a:<LAST-VERSION>'

    //适配minsdkversion >= 21 的版本,与>=16版本冲突
    //compile 'com.xl.media.library:xl-player-armeabi-21:<LAST-VERSION>'
    //compile 'com.xl.media.library:xl-player-armv7a-21:<LAST-VERSION>'
    //compile 'com.xl.media.library:xl-player-arm64v8a-21:<LAST-VERSION>'

建议三个abi版本的库都添加,以提高app性能.
如果为了app包大小考虑,可只添加:

    compile 'com.xl.media.library:xl-player-java:<LAST-VERSION>'
    compile 'com.xl.media.library:xl-player-armeabi:<LAST-VERSION>'

有时候第三方的sdk并没有提供armeabi版本的sdk,只提供了armv7a的sdk,则需要替换为

    compile 'com.xl.media.library:xl-player-java:<LAST-VERSION>'
    compile 'com.xl.media.library:xl-player-armv7a:<LAST-VERSION>'

以防止app报找不到第三方sdk.so的错误.

## [api说明](https://github.com/xl-player-developers/xl_player/wiki)

## [DemoApk下载](simple_apk/app-debug.apk)

## Demo说明

[SimpleDemoActivity](app/src/main/java/com/cls/xl/xl/YoutubeLikeActivity.java) 一个最基础的播放器使用页面

[SinglePlayerActivity](app/src/main/java/com/cls/xl/xl/SinglePlayerActivity.java) 播放音/视频的主要界面,里面涉及了大部分播放器的使用逻辑

[SimpleVideoActivity](app/src/main/java/com/cls/xl/xl/SimpleVideoActivity.java) 一个简单的Demo视频列表

[MultiPlayerActivity](app/src/main/java/com/cls/xl/xl/MultiPlayerActivity.java) 列表形式的播放器页面

[ChooseFileActivity](app/src/main/java/com/cls/xl/xl/ChooseFileActivity.java) 文件选择页面,可以选择本地音视频文件进行播放

[YoutubeLikeActivity](app/src/main/java/com/cls/xl/xl/YoutubeLikeActivity.java) 类似youtube的播放页面

[WhackAMoleActivity](app/src/main/java/com/cls/xl/xl/YoutubeLikeActivity.java) 打地鼠(播放中无缝切换Surface)页面



