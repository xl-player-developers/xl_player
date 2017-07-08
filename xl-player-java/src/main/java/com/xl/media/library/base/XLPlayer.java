package com.xl.media.library.base;

import android.content.Context;
import android.media.AudioManager;
import android.view.Surface;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Locale;

import static com.xl.media.library.base.XLPlayer.MODEL_TYPE.Rect;

public class XLPlayer {

    private OnErrorCodeListener onErrorCodeListener;
    private OnPlayerStatusChangeListener onPlayerStatusChangeListener;
    private Surface surface;
    private int oldStatus = -1;
    private MODEL_TYPE modelType;

    public enum MODEL_TYPE {
        Rect(0), Ball(1), VR(2), Planet(3), Architecture(4), Expand(5);
        private int value;

        MODEL_TYPE(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }
    }

    public class Statistics {
        private int fps;
        private int bps;
        private int bufferLength;

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


    public XLPlayer(Context context) {
        int bestrate = 44100;
        if (android.os.Build.VERSION.SDK_INT >= 17) {
            AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            String rate = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            bestrate = Integer.parseInt(rate);
        }
        BaseNativeInterface.initPlayer(this, android.os.Build.VERSION.SDK_INT, bestrate);
    }

    public void playVideo(String url, float time, MODEL_TYPE model) {
        modelType = model;
        int ret = BaseNativeInterface.play(url, time, model.getValue());
        if (ret != 0 && onErrorCodeListener != null) {
            onErrorCodeListener.onGetErrorCode(ret);
        }
    }


    public void playVideo(String url, int time) {
        playVideo(url, time, Rect);
    }

    public void playVideo(String url) {
        playVideo(url, 0, Rect);
    }

    public void seekTo(float time) {
        BaseNativeInterface.seek(time, true);
    }

    public void seekTime(float time) {
        BaseNativeInterface.seek(time, false);
    }

    public void pauseVideo() {
        BaseNativeInterface.pause();
    }

    public void resumeVideo() {
        BaseNativeInterface.resume();
    }

    public void resize(int w, int h) {
        BaseNativeInterface.resize(w, h);
    }

    public void stopVideo() {
        BaseNativeInterface.stop();
    }

    public void rotate(boolean clockwise) {
        BaseNativeInterface.rotate(clockwise);
    }

    public float getVideoTotalTime() {
        return BaseNativeInterface.getTotalTime();
    }

    public float getVideoCurrentTime() {
        return BaseNativeInterface.getCurrentTime();
    }

    public void releaseVideo() {
        BaseNativeInterface.release();
    }

    public void setForceSwDecode(boolean forceSwDecode) {
        BaseNativeInterface.setForceSwDecode(forceSwDecode);
    }

    public void setRate(float rate) {
        BaseNativeInterface.changeRate(rate);
    }

    public void setEnableTracker(boolean enableTracker) {
        BaseNativeInterface.setHeadTrackerEnable(enableTracker);
    }

    public boolean getEnableTracker() {
        return BaseNativeInterface.getHeadTrackerEnable();
    }

    public Surface getSurface() {
        return surface;
    }


    public void changeModel(MODEL_TYPE model) {
        modelType = model;
        BaseNativeInterface.changeModel(model.getValue());
    }

    public MODEL_TYPE getModelType() {
        return modelType;
    }

    public void setSurface(Surface xlsurface) {
        removeSurface();
        BaseNativeInterface.setSurface(xlsurface);
        this.surface = xlsurface;
    }

    public void removeSurface() {
        if (this.surface != null) {
            this.surface = null;
        }
    }

    public void setOnPlayerStatusChangeListener(OnPlayerStatusChangeListener onPlayerStatusChangeListener) {
        this.onPlayerStatusChangeListener = onPlayerStatusChangeListener;
    }

    public void setOnErrorCodeListener(OnErrorCodeListener onErrorCodeListener) {
        this.onErrorCodeListener = onErrorCodeListener;
    }

    public void onPause() {
        if (BaseNativeInterface.getPlayStatus() > 0) {
            pauseVideo();
        }
    }

    public void onResume() {
        if (BaseNativeInterface.getPlayStatus() > 0) {
            resumeVideo();
        }
    }

    public void setScale(float scale) {
        BaseNativeInterface.setScale(scale);
    }

    public void setRotation(float rx, float ry, float rz) {
        BaseNativeInterface.setRotation(rx, ry, rz);
    }

    public Statistics getStatistics() {
        return new Statistics(BaseNativeInterface.getStatistics());
    }

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

    void onPlayError(int error) {
        if (onErrorCodeListener != null){
            onErrorCodeListener.onGetErrorCode(error);
        }
        System.out.println(error);
    }
}
