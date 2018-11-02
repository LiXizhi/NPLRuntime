using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using cAudio;

namespace cAudioPlayer
{
    public partial class MainWnd : Form
    {
        private IAudioManager mAudioMgr;
        private IAudioSource mCurrentSource;
        private readonly AudioSourceListener mAudioListener;
        private readonly Timer mUpdateTimer = new Timer();
        public MainWnd()
        {
            InitializeComponent();
            mAudioMgr = cAudioCSharpWrapper.createAudioManager(true);
            mAudioListener = new AudioSourceListener(this);
            mUpdateTimer.Tick += UpdateTimerTick;
            Closing += MainWnd_Closing;
        }

        void MainWnd_Closing(object sender, CancelEventArgs e)
        {
            if (mCurrentSource != null)
            {
                mCurrentSource.unRegisterEventHandler(mAudioListener);
                mCurrentSource = null;
            }
            mAudioMgr.Dispose();
            mAudioMgr = null;
        }

        void UpdateTimerTick(object sender, EventArgs e)
        {
            UpdateSlider();
        }

        private void BtnOpenClick(object sender, EventArgs e)
        {
            btnPlay.Text = "Play";
            btnPlay.Enabled = false;
            btnStop.Enabled = false;
            currentPostion.Value = 0;
            mUpdateTimer.Stop();
            
            if (mCurrentSource != null)
            {
                mCurrentSource.unRegisterEventHandler(mAudioListener);
                mAudioMgr.release(mCurrentSource);
                mCurrentSource = null;
            }

            var openFileDlg = new OpenFileDialog();
            if (openFileDlg.ShowDialog() == DialogResult.OK)
            {
                var file = openFileDlg.FileName;
                mCurrentSource = mAudioMgr.create("musicFile", file);
                mCurrentSource.registerEventHandler(mAudioListener);
                fileNFO.Text = "File:" + file.Substring(file.LastIndexOf("\\") + 1) + " | Length: " + (mCurrentSource.getTotalAudioTime() / 60).ToString("#.00") + " min";
                btnPlay.Enabled = true;
            }

        }

        private void BtnPlayClick(object sender, EventArgs e)
        {
            btnStop.Enabled = true;
            if(btnPlay.Text == "Play")
            {
                btnPlay.Text = "Pause";
                mCurrentSource.play2d(false);

                mUpdateTimer.Interval = 50;
                mUpdateTimer.Start();
            }
            else
            {
                btnPlay.Text = "Play";
                mCurrentSource.pause();
                mUpdateTimer.Stop();
            }
        }

        private void BtnStopClick(object sender, EventArgs e)
        {
            OnStop();
        }


        public void OnStop()
        {
            btnPlay.Invoke((MethodInvoker)delegate { btnPlay.Text = "Play"; });
            btnStop.Invoke((MethodInvoker)delegate { btnStop.Enabled = false; });

            if(mCurrentSource.isPlaying())
                mCurrentSource.stop();

            mUpdateTimer.Stop();
            currentPostion.Invoke((MethodInvoker)delegate { currentPostion.Value = 0; });
        }

        public void UpdateSlider()
        {
            var percent = (int)((mCurrentSource.getCurrentAudioTime() / mCurrentSource.getTotalAudioTime()) * 1000);
            if (currentPostion.InvokeRequired)
            {
                currentPostion.Invoke((MethodInvoker)delegate { currentPostion.Value = percent; });
            }
            else
            {
                currentPostion.Value = percent;
            }
        }
    }
}
