package com.xl.media.library.base;

import android.content.Context;
import android.media.AudioManager;
import android.view.Surface;

import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Locale;

import static com.xl.media.library.base.XLPlayer.MODEL_TYPE.Rect;

/**
 * XLPlayer
 */
public class XLPlayer implements Serializable {

    private OnErrorCodeListener onErrorCodeListener;
    private OnPlayerStatusChangeListener onPlayerStatusChangeListener;
    private Surface surface;
    private int oldStatus = -1;
    private MODEL_TYPE modelType;

    /**
     * 播放器画面模型枚举
     */
    public enum MODEL_TYPE implements Serializable {

        Rect(0),//矩形
        Ball(1),//球型
        VR(2),//双眼
        Planet(3),//小行星
        Architecture(4),//建筑
        Expand(5);//展开
        private int value;

        MODEL_TYPE(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }
    }

    /**
     * 播放器动态参数
     */
    public class Statistics implements Serializable {
        private int fps;//帧率
        private int bps;//下载速度
        private int bufferLength;//缓存时长

        public Statistics(ByteBuffer buffer) {
            if (buffer != null) {
                buffer.order(ByteOrder.BIG_ENDIAN);
                buffer.position(0);
                fps = buffer.getInt();
                bps = buffer.getInt();
                bufferLength = buffer.getInt();
            }
        }

        public String getFormatBps() {
            if (bps > 1000000) {
                return String.format(Locale.getDefault(), "%.2fMb/s", (float) bps / 1000000f);
            } else if (bps > 1000) {
                return String.format(Locale.getDefault(), "%.2fKb/s", (float) bps / 1000f);
            }
            return String.format(Locale.getDefault(), "%dKb/s", bps);
        }

        public int getBps() {
            return bps;
        }

        public int getFps() {
            return fps;
        }

        public int getBufferLength() {
            return bufferLength;
        }
    }

    /**
     * 实例化一个播放器对象
     *
     * @param context context
     */
    public XLPlayer(Context context) {
        int bestrate = 44100;
        if (android.os.Build.VERSION.SDK_INT >= 17) {
            AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            String rate = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            bestrate = Integer.parseInt(rate);
        }
        BaseNativeInterface.initPlayer(this, android.os.Build.VERSION.SDK_INT, bestrate);
    }

    /**
     * 从某个时间开始以某种画面模型播放视频
     *
     * @param url   视频地址
     * @param time  起始时间
     * @param model 模型类型
     */
    public void playVideo(String url, float time, MODEL_TYPE model) {
        modelType = model;
        int ret = BaseNativeInterface.play(url, time, model.getValue());
        if (ret != 0 && onErrorCodeListener != null) {
            onErrorCodeListener.onGetErrorCode(ret);
        }
    }

    /**
     * 从某个时间开始播放一个视频
     *
     * @param url  视频地址
     * @param time 起始时间
     */
    public void playVideo(String url, int time) {
        playVideo(url, time, Rect);
    }

    /**
     * 以某种模型播放一个视频
     *
     * @param url       视频地址
     * @param modelType 模型类型
     */
    public void playVideo(String url, MODEL_TYPE modelType) {
        this.modelType = modelType;
        playVideo(url, 0, modelType);
    }

    /**
     * 播放一个视频
     *
     * @param url 视频地址
     */
    public void playVideo(String url) {
        playVideo(url, 0, Rect);
    }

    /**
     * seek到某个时间播放
     *
     * @param time 要seek到的时间
     */
    public void seekTo(float time) {
        BaseNativeInterface.seek(time, true);
    }

    /**
     * 以当前播放时间为基准seek一定时长
     *
     * @param time 要seek的时长,正数向前,负数向后
     */
    public void seekTime(float time) {
        BaseNativeInterface.seek(time, false);
    }

    /**
     * 暂停播放
     */
    public void pauseVideo() {
        BaseNativeInterface.pause();
    }

    /**
     * 回复播放
     */
    public void resumeVideo() {
        BaseNativeInterface.resume();
    }

    /**
     * 设置画面大小
     *
     * @param w 画面宽度
     * @param h 画面高度
     */
    public void resize(int w, int h) {
        BaseNativeInterface.resize(w, h);
    }

    /**
     * 停止播放video
     */
    public void stopVideo() {
        BaseNativeInterface.stop();
    }

    /**
     * 按顺时针或者逆时针旋转画面90度
     *
     * @param clockwise true 顺时针旋转,false逆时针旋转
     */
    public void rotate(boolean clockwise) {
        BaseNativeInterface.rotate(clockwise);
    }

    /**
     * 获取视频总时长
     *
     * @return 单位是秒
     */
    public float getVideoTotalTime() {
        return BaseNativeInterface.getTotalTime();
    }

    /**
     * 回去当前播放时间
     *
     * @return 单位是秒
     */
    public float getVideoCurrentTime() {
        return BaseNativeInterface.getCurrentTime();
    }

    /**
     * release player
     */
    public void releasePlayer() {
        BaseNativeInterface.release();
    }

    /**
     * 设置是否强制开启软件
     *
     * @param forceSwDecode true 打开,false 关闭
     */
    public void setForceSwDecode(boolean forceSwDecode) {
        BaseNativeInterface.setForceSwDecode(forceSwDecode);
    }

    /**
     * 设置播放速率
     *
     * @param rate 默认为1.0
     */
    public void setRate(float rate) {
        BaseNativeInterface.changeRate(rate);
    }

    /**
     * 设置陀螺仪的开启和关闭
     *
     * @param enableTracker true 打开,false 关闭
     */
    public void setEnableTracker(boolean enableTracker) {
        BaseNativeInterface.setHeadTrackerEnable(enableTracker);
    }

    /**
     * 获取播放器陀螺仪是否开启
     *
     * @return true 打开,false 关闭
     */
    public boolean getEnableTracker() {
        return BaseNativeInterface.getHeadTrackerEnable();
    }

    /**
     * 获取播放器当前的surface
     *
     * @return {@link Surface}
     */
    public Surface getSurface() {
        return surface;
    }

    /**
     * 切换player的播放模型
     *
     * @param model {@link MODEL_TYPE}
     */
    public void changeModel(MODEL_TYPE model) {
        modelType = model;
        BaseNativeInterface.changeModel(model.getValue());
    }

    /**
     * 获取player当前的播放类型
     *
     * @return {@link MODEL_TYPE}
     */
    public MODEL_TYPE getModelType() {
        return modelType;
    }

    /**
     * 为player设置surface
     *
     * @param xlsurface 要为player设置的Surface
     */
    public void setSurface(Surface xlsurface) {
        removeSurface();
        BaseNativeInterface.setSurface(xlsurface);
        this.surface = xlsurface;
    }

    /**
     * 移除player中的surface
     */
    public void removeSurface() {
        if (this.surface != null) {
            this.surface = null;
        }
    }

    /**
     * 设置播放器状态切换回调
     *
     * @param onPlayerStatusChangeListener 状态回调{@link OnPlayerStatusChangeListener}
     */
    public void setOnPlayerStatusChangeListener(OnPlayerStatusChangeListener onPlayerStatusChangeListener) {
        this.onPlayerStatusChangeListener = onPlayerStatusChangeListener;
    }

    /**
     * 设置错误码回调
     *
     * @param onErrorCodeListener 错误码回调{@link OnErrorCodeListener}
     */
    public void setOnErrorCodeListener(OnErrorCodeListener onErrorCodeListener) {
        this.onErrorCodeListener = onErrorCodeListener;
    }

    /**
     * pauseVideo调用,添加了播放器状态判断
     */
    public void onPause() {
        if (BaseNativeInterface.getPlayStatus() > 0) {
            pauseVideo();
        }
    }

    /**
     * resumeVideo调用,添加了播放器状态判断
     */
    public void onResume() {
        if (BaseNativeInterface.getPlayStatus() > 0) {
            resumeVideo();
        }
    }

    /**
     * 设置缩放
     *
     * @param scale 缩放大小比例[0.5-2.0]
     */
    public void setScale(float scale) {
        BaseNativeInterface.setScale(scale);
    }

    /**
     * 设置3D旋转
     *
     * @param rx x轴旋转弧度
     * @param ry y轴旋转弧度
     * @param rz z轴旋转弧度
     */
    public void setRotation(float rx, float ry, float rz) {
        BaseNativeInterface.setRotation(rx, ry, rz);
    }

    /**
     * 返回播放信息
     *
     * @return {@link XLPlayer.Statistics}
     */
    public Statistics getStatistics() {
        return new Statistics(BaseNativeInterface.getStatistics());
    }

    /**
     * 设置是否后台播放
     *
     * @param playBackground true 后台播放,false 关闭后台播放,默认为false
     */
    public void setPlayBackground(boolean playBackground) {
        BaseNativeInterface.setPlayBackground(playBackground);
    }

    /**
     * 设置缓存时长,单位:秒
     * 默认5s
     * buffer time 和 buffer size 任意一个满了就停止读取数据
     *
     * @param seconds 缓存时长
     */
    public void setBufferTime(float seconds) {
        BaseNativeInterface.setBufferTime(seconds);
    }


    /**
     * 设置缓存大小,单位:字节
     * 默认5MB
     * buffer time 和 buffer size 任意一个满了就停止读取数据
     *
     * @param bytes 缓存大小
     */
    public void setBufferSize(int bytes) {
        BaseNativeInterface.setBufferSize(bytes);
    }

    @SuppressWarnings("unused")
    void onPlayStatusChanged(int status) {
        switch (status) {
            case 0:
                if (oldStatus == -1) {
                    if (this.onPlayerStatusChangeListener != null) {
                        onPlayerStatusChangeListener.onPrepared();
                    }
                } else if (oldStatus > 0) {
                    // TODO: 2017/5/22 判断错误码
                    if (this.onPlayerStatusChangeListener != null) {
                        onPlayerStatusChangeListener.onStop();
                    }
                }
                break;
            case 1:
                if (this.onPlayerStatusChangeListener != null) {
                    if (oldStatus == 0) {
                        onPlayerStatusChangeListener.onStart();
                    } else {
                        onPlayerStatusChangeListener.onResume();
                    }
                }
                break;
            case 2:
                if (this.onPlayerStatusChangeListener != null) {
                    onPlayerStatusChangeListener.onPause();
                }
                break;
            case 3:
                if (this.onPlayerStatusChangeListener != null) {
                    onPlayerStatusChangeListener.onBufferEmpty();
                }
                break;
            case 4:
                if (this.onPlayerStatusChangeListener != null) {
                    onPlayerStatusChangeListener.onBufferFull();
                }
                break;
        }

        oldStatus = status;
    }

    /**
     * native 调用的错误码回调
     *
     * @param error error code from native
     */
    void onPlayError(int error) {
        if (onErrorCodeListener != null) {
            onErrorCodeListener.onGetErrorCode(error);
        }
    }
}
