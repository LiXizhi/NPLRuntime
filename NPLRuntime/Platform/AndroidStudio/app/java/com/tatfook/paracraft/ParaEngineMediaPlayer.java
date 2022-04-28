package com.tatfook.paracraft;

import android.media.MediaDataSource;
import android.media.MediaPlayer;

import java.io.IOException;

public class ParaEngineMediaPlayer {
    static private byte[] mMidiFileData = {0x4d, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x78, 0x4d, 0x54, 0x72, 0x6b, 0x00, 0x00, 0x00,0x0c, 0x00, (byte)0x90, 0x48, 0x40, 0x78, (byte)0x80, 0x48, 0x40, 0x00, (byte)0xff, 0x2f, 0x00};
    static MediaPlayer mMediaPlayer = new MediaPlayer();
    public static void PlayMidiNote(int note, int velocity) {
        mMidiFileData[24] = (byte)note;
        mMidiFileData[25] = (byte)velocity;
        mMidiFileData[28] = (byte)note;
        mMidiFileData[29] = (byte)velocity;

        mMediaPlayer.reset();
        mMediaPlayer.setDataSource(new MidiNoteMediaDataSource());
        try {
            mMediaPlayer.prepare();
        } catch (IOException e) {
            e.printStackTrace();
        }
        mMediaPlayer.start();
    }

    public static void SetVolume(float volume) {
        mMediaPlayer.setVolume(volume, volume);
    }

    public static void PlayMidiFile(String filename) {
        try {
            mMediaPlayer.setDataSource(filename);
        } catch (IOException e) {
            e.printStackTrace();
            return ;
        }

        try {
            mMediaPlayer.prepare();
            mMediaPlayer.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    //    public static void SetVelocity(byte vec)
    static class MidiNoteMediaDataSource extends MediaDataSource {
        @Override
        public int readAt(long position, byte[] buffer, int offset, int size) throws IOException {
            int count = 0;
            for (int i = 0; i < size; i++) {
                if ((position + i) >= mMidiFileData.length) break;
                buffer[offset + i] = mMidiFileData[(int)(position + i)];
                count++;
            }
            return count;
        }

        @Override
        public long getSize() throws IOException {
            return mMidiFileData.length;
        }

        @Override
        public void close() throws IOException {
        }
    }
}
