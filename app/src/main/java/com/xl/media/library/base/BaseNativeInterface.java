package com.xl.media.library.base;

import android.content.res.AssetManager;
import android.view.Surface;

import java.nio.ByteBuffer;


class BaseNativeInterface {
    static {
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("xl_render");
    }


    /**
     * XLSurface jni
     */
    native static void initPlayer(XLPlayer xlPlayer, AssetManager manager,int runAndroidVersion,int bestSampleRate);

    native static void setBufferTime(float time);

    native static void setBufferSize(int numByte);

    native static void setPlayBackground(boolean playBackground);

    native static void setForceSwDecode(boolean forceSwDecode);

    native static void setSurface(Surface surface);

    static native void resize(int width, int height);

    static native void changeRate(float rate);

    native static int play(String url, float time, int model);

    native static void seek(float time, boolean isSeekTo);

    native static void pause();

    native static void resume();

    native static void stop();

    native static void rotate(boolean clockwise);

    native static float getTotalTime();

    native static float getCurrentTime();

    native static void release();

    native static int getPlayStatus();

    native static void changeModel(int model);

    native static void setScale(float scale);

    native static void setRotation(float rx, float ry, float rz);

    native static void setHeadTrackerEnable(boolean enable);

    native static boolean getHeadTrackerEnable();

    native static ByteBuffer getStatistics();

}
