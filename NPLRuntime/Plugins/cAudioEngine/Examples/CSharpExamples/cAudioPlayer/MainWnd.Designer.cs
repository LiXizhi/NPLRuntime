namespace cAudioPlayer
{
    partial class MainWnd
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.fileNFO = new System.Windows.Forms.Label();
            this.btnOpen = new System.Windows.Forms.Button();
            this.currentPostion = new System.Windows.Forms.TrackBar();
            this.btnPlay = new System.Windows.Forms.Button();
            this.btnStop = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.currentPostion)).BeginInit();
            this.SuspendLayout();
            // 
            // fileNFO
            // 
            this.fileNFO.AutoSize = true;
            this.fileNFO.Location = new System.Drawing.Point(51, 9);
            this.fileNFO.Name = "fileNFO";
            this.fileNFO.Size = new System.Drawing.Size(46, 13);
            this.fileNFO.TabIndex = 0;
            this.fileNFO.Text = "File: n/a";
            // 
            // btnOpen
            // 
            this.btnOpen.Location = new System.Drawing.Point(2, 4);
            this.btnOpen.Name = "btnOpen";
            this.btnOpen.Size = new System.Drawing.Size(43, 23);
            this.btnOpen.TabIndex = 1;
            this.btnOpen.Text = "Open";
            this.btnOpen.UseVisualStyleBackColor = true;
            this.btnOpen.Click += new System.EventHandler(this.BtnOpenClick);
            // 
            // currentPostion
            // 
            this.currentPostion.Location = new System.Drawing.Point(2, 33);
            this.currentPostion.Maximum = 1000;
            this.currentPostion.Name = "currentPostion";
            this.currentPostion.Size = new System.Drawing.Size(492, 45);
            this.currentPostion.TabIndex = 2;
            this.currentPostion.TickStyle = System.Windows.Forms.TickStyle.Both;
            // 
            // btnPlay
            // 
            this.btnPlay.Location = new System.Drawing.Point(2, 84);
            this.btnPlay.Name = "btnPlay";
            this.btnPlay.Size = new System.Drawing.Size(43, 23);
            this.btnPlay.TabIndex = 3;
            this.btnPlay.Text = "Play";
            this.btnPlay.UseVisualStyleBackColor = true;
            this.btnPlay.Click += new System.EventHandler(this.BtnPlayClick);
            // 
            // btnStop
            // 
            this.btnStop.Location = new System.Drawing.Point(51, 84);
            this.btnStop.Name = "btnStop";
            this.btnStop.Size = new System.Drawing.Size(43, 23);
            this.btnStop.TabIndex = 4;
            this.btnStop.Text = "Stop";
            this.btnStop.UseVisualStyleBackColor = true;
            this.btnStop.Click += new System.EventHandler(this.BtnStopClick);
            // 
            // MainWnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(498, 113);
            this.Controls.Add(this.btnStop);
            this.Controls.Add(this.btnPlay);
            this.Controls.Add(this.currentPostion);
            this.Controls.Add(this.btnOpen);
            this.Controls.Add(this.fileNFO);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "MainWnd";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "cAudio C# Player";
            ((System.ComponentModel.ISupportInitialize)(this.currentPostion)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label fileNFO;
        private System.Windows.Forms.Button btnOpen;
        private System.Windows.Forms.TrackBar currentPostion;
        private System.Windows.Forms.Button btnPlay;
        private System.Windows.Forms.Button btnStop;
    }
}

