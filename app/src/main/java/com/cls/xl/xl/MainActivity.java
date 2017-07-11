package com.cls.xl.xl;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.cls.xl.xl.youtube.YoutubeLikeActivity;

import java.util.ArrayList;

public class MainActivity extends Activity {
    ArrayList<Intent> playerName = new ArrayList<>();
    RecyclerView choosePlayerList;
    public static boolean forceSwDecode = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initData();
        choosePlayerList = (RecyclerView) findViewById(R.id.choose_player);
        choosePlayerList.setLayoutManager(new LinearLayoutManager(this));
        choosePlayerList.setAdapter(new ChooseAdapter());
        findViewById(R.id.play_video).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String mediaUrl = ((EditText) findViewById(R.id.url_edit)).getText().toString().trim();
                if (mediaUrl.isEmpty()) {
                    Toast.makeText(MainActivity.this, "请输入正确的地址", Toast.LENGTH_SHORT).show();
                } else {
                    Intent intent = new Intent(MainActivity.this, SinglePlayerActivity.class);
                    intent.putExtra("media_url", mediaUrl);
                    startActivity(intent);
                }
            }
        });
        ((Switch) findViewById(R.id.open_sw)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                forceSwDecode = isChecked;
            }
        });
    }

    private void initData() {
        Intent intent;
        intent = new Intent(MainActivity.this, ChooseFileActivity.class);
        intent.setAction("Choose file");
        playerName.add(intent);
        intent = new Intent(MainActivity.this, SampleVideoActivity.class);
        intent.setAction("Simple video");
        playerName.add(intent);
        intent = new Intent(MainActivity.this, MultiPlayerActivity.class);
        intent.setAction("Multi player");
        playerName.add(intent);
        intent = new Intent(MainActivity.this, YoutubeLikeActivity.class);
        intent.setAction("Youtube like");
        playerName.add(intent);
        intent = new Intent(MainActivity.this, WhackAMoleActivity.class);
        intent.setAction("whack a mole");
        playerName.add(intent);
        intent = new Intent(MainActivity.this, SimpleDemoActivity.class);
        intent.setAction("SimpleDemo");
        playerName.add(intent);
    }

    class ChooseAdapter extends RecyclerView.Adapter<ChooseViewHolder> {

        @Override
        public ChooseViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            return new ChooseViewHolder(LayoutInflater.from(MainActivity.this).inflate(R.layout.item_player_page, null));
        }

        @Override
        public void onBindViewHolder(ChooseViewHolder holder, final int position) {
            holder.playerName.setText(playerName.get(position).getAction());
            holder.playerName.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    MainActivity.this.startActivity(playerName.get(position));
                }
            });
        }

        @Override
        public int getItemCount() {
            return playerName.size();
        }
    }

    class ChooseViewHolder extends RecyclerView.ViewHolder {
        TextView playerName;

        ChooseViewHolder(View itemView) {
            super(itemView);
            playerName = (TextView) itemView.findViewById(R.id.player_name);
        }
    }


}
