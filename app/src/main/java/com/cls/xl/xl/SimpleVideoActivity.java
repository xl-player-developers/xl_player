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
        videoUrls.put("TestVR", "http://cache.utovr.com/604b657d08cb4c60963ad3582aa8074a/L2_wixzbeossojxw88a.mp4");
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
