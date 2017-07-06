package com.cls.xl.xl;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.Collections;


public class ChooseFileActivity extends Activity {
    RecyclerView fileList;
    ArrayList<File> files = new ArrayList<>();
    ArrayList<File> filesF = new ArrayList<>();
    File[] filesA;
    File nowDirectory;
    FileAdapter adapter;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_choose_file);
        checkPermissions();
    }

    private void checkPermissions() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 10086);
        } else {
            initUI();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        switch (requestCode) {
            case 10086:
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    initUI();
                } else {
                    checkPermissions();
                }
                break;
            default:
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
                break;
        }
    }

    void initUI() {
        getFiles(Environment.getExternalStorageDirectory());
        fileList = (RecyclerView) findViewById(R.id.file_list);
        fileList.setLayoutManager(new LinearLayoutManager(this));
        adapter = new FileAdapter();
        fileList.setAdapter(adapter);
    }

    private void getFiles(File directoryFile) {
        files.clear();
        filesF.clear();
        if (!directoryFile.getAbsolutePath().equalsIgnoreCase(Environment.getExternalStorageDirectory().getParent())) {
            nowDirectory = directoryFile;
            filesA = nowDirectory.listFiles(new FileFilter() {
                @Override
                public boolean accept(File pathname) {
                    return !(pathname.getName().startsWith(".") || pathname.getName().trim().length() == 0);
                }
            });
            if (filesA != null) {
                for (File file : filesA) {
                    if (file.isDirectory()) {
                        files.add(file);
                    } else {
                        filesF.add(file);
                    }
                }
            }
            files.addAll(filesF);
        }
        Collections.sort(files);
    }

    class FileAdapter extends RecyclerView.Adapter<ChooseFileActivity.FileHolder> {

        @Override
        public ChooseFileActivity.FileHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            return new ChooseFileActivity.FileHolder(LayoutInflater.from(ChooseFileActivity.this).inflate(R.layout.item_file, parent, false));
        }

        @Override
        public void onBindViewHolder(final ChooseFileActivity.FileHolder holder, int position) {
            if (position == 0) {
                holder.fileType.setBackground(getResources().getDrawable(R.drawable.directory_icon));
                holder.fileName.setText("...");
                holder.itemView.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        getFiles(new File(nowDirectory.getParent()));
                        FileAdapter.this.notifyDataSetChanged();
                    }
                });
            } else {
                final File file = files.get(position - 1);
                holder.fileType.setBackground(getResources().getDrawable(file.isDirectory() ? R.drawable.directory_icon : R.drawable.file_icon));
                holder.fileName.setText(file.getName());
                holder.itemView.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if (file.isDirectory()) {
                            getFiles(file);
                            FileAdapter.this.notifyDataSetChanged();
                        } else {
                            Intent intent = new Intent(ChooseFileActivity.this, SinglePlayerActivity.class);
                            intent.putExtra("media_url", file.getAbsolutePath());
                            startActivity(intent);
                        }
                    }
                });
            }
        }

        @Override
        public int getItemCount() {
            return files.size() + 1;
        }
    }

    class FileHolder extends RecyclerView.ViewHolder {
        TextView fileName;
        ImageView fileType;

        FileHolder(View itemView) {
            super(itemView);
            fileName = (TextView) itemView.findViewById(R.id.file_name);
            fileType = (ImageView) itemView.findViewById(R.id.file_type);
        }
    }

    @Override
    public void onBackPressed() {
        if (nowDirectory.getAbsolutePath().equalsIgnoreCase(Environment.getExternalStorageDirectory().getAbsolutePath())) {
            super.onBackPressed();
        } else {
            getFiles(new File(nowDirectory.getParent()));
            adapter.notifyDataSetChanged();
        }
    }
}
