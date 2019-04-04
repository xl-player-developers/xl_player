package com.xl.media.library.base;

import java.io.Serializable;

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

