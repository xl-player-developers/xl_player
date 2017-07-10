package com.cls.xl.xl;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.xl.media.library.base.MODEL_TYPE;

import java.util.ArrayList;
import java.util.LinkedHashMap;

public class SimpleVideoActivity extends Activity {
    RecyclerView videoList;
    LinkedHashMap<String, String> videoUrls = new LinkedHashMap<>();
    ArrayList<String> nameList;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_simple_video);
        initData();
        videoList = (RecyclerView) findViewById(R.id.video_list);
        videoList.setLayoutManager(new LinearLayoutManager(this));
        videoList.setAdapter(new VideoAdapter());
    }

    void initData() {
        videoUrls.put("TestVR_card_board", "http://cache.utovr.com/275fb5545211474b9109b30abac9bc2b/L2_aoo1oouobumjfdfw.m3u8");
        videoUrls.put("TestVR_ball", "http://cache.utovr.com/275fb5545211474b9109b30abac9bc2b/L2_aoo1oouobumjfdfw.m3u8");
        videoUrls.put("TestVR_plant", "http://cache.utovr.com/275fb5545211474b9109b30abac9bc2b/L2_aoo1oouobumjfdfw.m3u8");
        videoUrls.put("TestVR_Architecture", "http://cache.utovr.com/s1cufkefpqsrztkde7/L2_3rv4jtr7u766p6d4.m3u8");
        videoUrls.put("TestVR_Expand", "http://cache.utovr.com/275fb5545211474b9109b30abac9bc2b/L2_aoo1oouobumjfdfw.m3u8");
//        videoUrls.put("rtmp", "rtmp://192.168.1.207:11935/live/biandroid");
//        videoUrls.put("hls", "http://192.168.1.207/hls0/x.m3u8");
        videoUrls.put("Android screens (Matroska)", "http://storage.googleapis.com/exoplayer-test-media-1/mkv/android-screens-lavf-56.36.100-aac-avc-main-1280x720.mkv");
        videoUrls.put("Google Glass (WebM Video with Vorbis Audio)", "http://demos.webmproject.org/exoplayer/glass_vp9_vorbis.webm");
        videoUrls.put("Google Glass (VP9 in MP4/ISO-BMFF)", "http://demos.webmproject.org/exoplayer/glass.mp4");
        videoUrls.put("Big Buck Bunny (FLV Video)", "http://vod.leasewebcdn.com/bbb.flv?ri=1024&rs=150&start=0");
        videoUrls.put("Google Play (MP3 Audio)", "http://storage.googleapis.com/exoplayer-test-media-0/play.mp3");
        nameList = new ArrayList<>(videoUrls.keySet());
    }


    class VideoAdapter extends RecyclerView.Adapter<SimpleVideoActivity.VideoHolder> {

        @Override
        public SimpleVideoActivity.VideoHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            return new SimpleVideoActivity.VideoHolder(LayoutInflater.from(SimpleVideoActivity.this).inflate(R.layout.item_video, parent, false));
        }

        @Override
        public void onBindViewHolder(final SimpleVideoActivity.VideoHolder holder, int position) {
            final String name = nameList.get(position);
            holder.videoName.setText(name);
            holder.itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Intent intent = new Intent(SimpleVideoActivity.this, SinglePlayerActivity.class);
                    intent.putExtra("media_url", videoUrls.get(name));
                    MODEL_TYPE type;
                    switch (holder.getLayoutPosition()) {
                        case 0:
                            type = MODEL_TYPE.VR;
                            break;
                        case 1:
                            type = MODEL_TYPE.Ball;
                            break;
                        case 2:
                            type = MODEL_TYPE.Planet;
                            break;
                        case 3:
                            type = MODEL_TYPE.Architecture;
                            break;
                        case 4:
                            type = MODEL_TYPE.Expand;
                            break;
                        default:
                            type = MODEL_TYPE.Rect;
                            break;

                    }
                    intent.putExtra("model", type);
                    startActivity(intent);
                }
            });
        }

        @Override
        public int getItemCount() {
            return videoUrls.size();
        }
    }

    class VideoHolder extends RecyclerView.ViewHolder {
        TextView videoName;

        VideoHolder(View itemView) {
            super(itemView);
            videoName = (TextView) itemView.findViewById(R.id.video_name);
        }
    }
}
