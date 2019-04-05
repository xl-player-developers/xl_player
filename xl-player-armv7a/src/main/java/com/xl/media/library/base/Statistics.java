package com.xl.media.library.base;

import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Locale;

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
        } else {
            return String.format(Locale.getDefault(), "%db/s", bps);
        }

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
