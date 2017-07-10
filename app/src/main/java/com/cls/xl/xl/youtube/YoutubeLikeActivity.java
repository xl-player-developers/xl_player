package com.cls.xl.xl.youtube;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.cls.xl.xl.MainActivity;
import com.cls.xl.xl.R;
import com.xl.media.library.base.XLPlayer;

import java.util.ArrayList;
import java.util.LinkedHashMap;

public class YoutubeLikeActivity extends Activity {
    LinkedHashMap<String, String> videoNames = new LinkedHashMap<>();
    ArrayList<String> nameList;
    RecyclerView videoList;
    XLPlayer player;
    SurfaceView floatSurface;
    DragVideoView mDragVideoView;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_youtube);
        player = new XLPlayer(this);
        player.setForceSwDecode(MainActivity.forceSwDecode);
        initData();
        videoList = (RecyclerView) findViewById(R.id.video_list);
        videoList.setLayoutManager(new LinearLayoutManager(YoutubeLikeActivity.this));
        videoList.setAdapter(new PlayerAdapter());
        floatSurface = (SurfaceView) findViewById(R.id.video_surface);
        floatSurface.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                player.setSurface(holder.getSurface());
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                player.resize(width, height);
                if (width == 540 && height == 303) {
                    videoList.setAlpha(1.0f);
                } else {
                    float f = (float) ((1.0 - ((float) width / 1080)) * 1.0f);
                    videoList.setAlpha(f);
                }
                videoList.setVisibility(View.VISIBLE);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });

        mDragVideoView = (DragVideoView) findViewById(R.id.drag_view);
        mDragVideoView.setCallback(mCallBack);
    }

    void initData() {
        videoNames.put("Android screens (Matroska)", "http://storage.googleapis.com/exoplayer-test-media-1/mkv/android-screens-lavf-56.36.100-aac-avc-main-1280x720.mkv");
        videoNames.put("Google Glass (WebM Video with Vorbis Audio)", "http://demos.webmproject.org/exoplayer/glass_vp9_vorbis.webm");
        videoNames.put("Google Glass (VP9 in MP4/ISO-BMFF)", "http://demos.webmproject.org/exoplayer/glass.mp4");
        videoNames.put("Big Buck Bunny (FLV Video)", "http://vod.leasewebcdn.com/bbb.flv?ri=1024&rs=150&start=0");
        videoNames.put("Google Play (MP3 Audio)", "http://storage.googleapis.com/exoplayer-test-media-0/play.mp3");
        nameList = new ArrayList<>(videoNames.keySet());
    }

    DragVideoView.Callback mCallBack = new DragVideoView.Callback() {
        @Override
        public void onDisappear(int direct) {
            player.stopVideo();
        }
    };

    class PlayerAdapter extends RecyclerView.Adapter<PlayerViewHolder> {

        @Override
        public PlayerViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            return new PlayerViewHolder(LayoutInflater.from(YoutubeLikeActivity.this).inflate(R.layout.item_video_name, parent, false));
        }

        @Override
        public void onBindViewHolder(final PlayerViewHolder holder, final int position) {
            final String name = nameList.get(position);
            holder.videoName.setText(name);
            holder.itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    mDragVideoView.show();
                    player.stopVideo();
                    player.playVideo(videoNames.get(name));
                }
            });
        }

        @Override
        public int getItemCount() {
            return videoNames.size();
        }

    }

    class PlayerViewHolder extends RecyclerView.ViewHolder {
        TextView videoName;

        PlayerViewHolder(View itemView) {
            super(itemView);
            videoName = (TextView) itemView.findViewById(R.id.video_name);
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        player.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        player.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        player.releasePlayer();
    }
}
