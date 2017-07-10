package com.cls.xl.xl;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.xl.media.library.base.XLPlayer;

import java.util.ArrayList;
import java.util.LinkedHashMap;

public class MultiPlayerActivity extends Activity {
    LinkedHashMap<String, String> videoNames = new LinkedHashMap<>();
    ArrayList<String> nameList;
    RecyclerView videoList;
    XLPlayer player;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_multi_player);
        player = new XLPlayer(this);
        player.setForceSwDecode(MainActivity.forceSwDecode);
        initData();
        videoList = (RecyclerView) findViewById(R.id.video_list);
        videoList.setLayoutManager(new LinearLayoutManager(MultiPlayerActivity.this));
        videoList.setAdapter(new PlayerAdapter());
    }

    void initData() {
        videoNames.put("Android screens (Matroska)", "http://storage.googleapis.com/exoplayer-test-media-1/mkv/android-screens-lavf-56.36.100-aac-avc-main-1280x720.mkv");
        videoNames.put("Google Glass (WebM Video with Vorbis Audio)", "http://demos.webmproject.org/exoplayer/glass_vp9_vorbis.webm");
        videoNames.put("Google Glass (VP9 in MP4/ISO-BMFF)", "http://demos.webmproject.org/exoplayer/glass.mp4");
        videoNames.put("Big Buck Bunny (FLV Video)", "http://vod.leasewebcdn.com/bbb.flv?ri=1024&rs=150&start=0");
        videoNames.put("Google Play (MP3 Audio)", "http://storage.googleapis.com/exoplayer-test-media-0/play.mp3");
        nameList = new ArrayList<>(videoNames.keySet());
    }

    class PlayerAdapter extends RecyclerView.Adapter<PlayerViewHolder> {

        @Override
        public PlayerViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            return new PlayerViewHolder(LayoutInflater.from(MultiPlayerActivity.this).inflate(R.layout.item_player, parent, false));
        }

        @Override
        public void onBindViewHolder(final PlayerViewHolder holder, int position) {
            final String name = nameList.get(position);
            holder.videoName.setText(name);
            holder.playVideo.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    player.stopVideo();
                    player.setSurface(new Surface(holder.xlPlayerView.getSurfaceTexture()));
                    player.playVideo(videoNames.get(name));
                    holder.xlPlayerView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
                        @Override
                        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {

                        }

                        @Override
                        public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {

                        }

                        @Override
                        public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
                            player.stopVideo();
                            holder.xlPlayerView.setSurfaceTextureListener(null);
                            return true;
                        }

                        @Override
                        public void onSurfaceTextureUpdated(SurfaceTexture surface) {

                        }
                    });
                }
            });
            holder.stopVideo.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    player.stopVideo();
                    player.removeSurface();
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
        ImageView playVideo, stopVideo;
        TextureView xlPlayerView;

        PlayerViewHolder(View itemView) {
            super(itemView);
            videoName = (TextView) itemView.findViewById(R.id.video_name);
            playVideo = (ImageView) itemView.findViewById(R.id.play_video);
            xlPlayerView = (TextureView) itemView.findViewById(R.id.player);
            stopVideo = (ImageView) itemView.findViewById(R.id.stop_video);
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
