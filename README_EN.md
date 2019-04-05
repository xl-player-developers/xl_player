# XLPlayer
A high performance Android media player, base on ffmpeg and MediaCodec, support VR video.
## Introduction

XLPlayer support almost all of media formats. Includ .mp4 .mkv .flv rtmp hls .webm .mov and so on.
When video coded by H263/H264/H265/MPEG4/VP8/VP9, XLPlayer will use MediaCodec decode video stream(some device not support H265 VP9).
Audio stream will use libavcodec decode, and use libavfilter resample to adapt to different device's best audio sample rate.

XLPlayer support some model types to rend VR video, like ball_model / google_card_board / little_plant / expand_model.
ball_model and google_card_board support gyroscope control.

XLPlayer support Variable Speed Playback, you can set playback with 0.5x-2.0x speed.

As usual XLPlayer work in MediaCodec->SurfaceTexture->OpenGL ES mode, this mode has high performance.

##  Instructions and Notes Please refer to [release_v1.0](https://github.com/xl-player-developers/xl_player/tree/release_v1.0)

#### Gradle configuration： implementation 'com.xl.media.library:xl-player-armv7a:2.0.1'

## v_2.0 Plan
* 1.Since the Ndk-abi version of the Android model is now very unified, more than 99% support armeabi-v7a, so in the new version, unnecessary abi version support logic will be removed to simplify the project structure. **（Done）**
* 2.The existing cache refers to the memory cache. The new version has plans to join the local cache logic. The cached media formats include but are not limited to normal streaming media, HLS, DASH.**（Not sure when to have time to do it, but within the plan）**
* 3.Separate branch to make a pure player, there is no VR and other functions pure player, because the current master version because of the need for VR and other functions make the project eventually larger.**（Not sure when to have time to do it, but within the plan）**
* 4.Audio playback is added to aaudio.**（Not sure when to have time to do it, but within the plan）**

## New version considerations
   **Only the armeabi-v7a is retained in the new version. It can be seen from [Tencent Statistics](https://mta.qq.com/mta/data/device/os) that the devices below api-21 account for more than 7%. In order to be able to adapt more models, the lowest and target versions in the project are api-16, so the mediandk in jni will not be called by default, only the mediacodec in java will be reflected.**
  
   **But the logic to call AMediaCodec is saved, so if your app version minsdk >= 21, you can change the minSdkVersion and targetSdkVersion in [build.gradle](xl-player-armv7a/build.gradle) to 21+ To enable this part of the logic.**
   
   **In the future, if the proportion of devices below api-21 falls below 0.1, the minSdkVersion and targetSdkVersion will be changed to 21 to enable AMediaCodec related logic.**
  
## examples

![图片1](sample_pic/1.gif)

![图片2](sample_pic/2.gif)

![图片3](sample_pic/3.gif)

![图片4](sample_pic/4.gif)

![图片5](sample_pic/5.gif)

![图片6](sample_pic/6.gif)
