package com.xl.media.library.base;

/**
 * 播放器返回错误码回调
 */
public interface OnErrorCodeListener {
    /**
     * #define XL_ERROR_AUDIO_DECODE_SEND_PACKET 3001
     * #define XL_ERROR_AUDIO_DECODE_CODEC_NOT_OPENED 3002
     * #define XL_ERROR_AUDIO_DECODE_RECIVE_FRAME 3003
     * <p>
     * #define XL_ERROR_VIDEO_SW_DECODE_SEND_PACKET 4101
     * #define XL_ERROR_VIDEO_SW_DECODE_CODEC_NOT_OPENED 4102
     * #define XL_ERROR_VIDEO_SW_DECODE_RECIVE_FRAME 4103
     * <p>
     * #define XL_ERROR_VIDEO_HW_MEDIACODEC_RECEIVE_FRAME 501
     *
     * @param errorCode 返回的错误码
     */
    void onGetErrorCode(int errorCode);
}
