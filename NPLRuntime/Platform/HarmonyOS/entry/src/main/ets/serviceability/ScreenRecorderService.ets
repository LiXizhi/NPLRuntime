/**
 * Screen Recorder Service for HarmonyOS
 * 对应Android项目中的ScreenRecorderService
 */

import Want from '@ohos.app.ability.Want';
import hilog from '@ohos.hilog';
import media from '@ohos.multimedia.media';
import { BusinessError } from '@ohos.base';

// 使用普通类而不是ServiceExtensionAbility，避免模块依赖问题
export default class ScreenRecorderService {
  private static readonly TAG = 'ScreenRecorderService';
  private recorder: media.AVRecorder | null = null;
  private isRecording: boolean = false;

  onCreate(want: Want): void {
    hilog.info(0x0000, ScreenRecorderService.TAG, 'ScreenRecorderService onCreate');
  }

  onDestroy(): void {
    hilog.info(0x0000, ScreenRecorderService.TAG, 'ScreenRecorderService onDestroy');
    if (this.isRecording) {
      this.stopRecording();
    }
  }

  onRequest(want: Want, startId: number): void {
    hilog.info(0x0000, ScreenRecorderService.TAG, 'ScreenRecorderService onRequest');
    
    const action = want.parameters?.['action'] as string;
    switch (action) {
      case 'start':
        this.startRecording();
        break;
      case 'stop':
        this.stopRecording();
        break;
      case 'pause':
        this.pauseRecording();
        break;
      case 'resume':
        this.resumeRecording();
        break;
    }
  }

  /**
   * 开始录制
   */
  private async startRecording(): Promise<void> {
    if (this.isRecording) {
      hilog.warn(0x0000, ScreenRecorderService.TAG, 'Already recording');
      return;
    }

    try {
      // 创建录制器
      this.recorder = await media.createAVRecorder();
      
      // 配置录制参数
      const config: media.AVRecorderConfig = {
        audioSourceType: media.AudioSourceType.AUDIO_SOURCE_TYPE_MIC,
        videoSourceType: media.VideoSourceType.VIDEO_SOURCE_TYPE_SURFACE_YUV,
        profile: {
          audioBitrate: 48000,
          audioChannels: 2,
          audioCodec: media.CodecMimeType.AUDIO_AAC,
          audioSampleRate: 48000,
          fileFormat: media.ContainerFormatType.CFT_MPEG_4,
          videoBitrate: 2000000,
          videoCodec: media.CodecMimeType.VIDEO_AVC,
          videoFrameWidth: 1920,
          videoFrameHeight: 1080,
          videoFrameRate: 30
        },
        url: `file:///data/storage/el2/base/haps/entry/files/screen_record_${Date.now()}.mp4`, // 硬编码路径，因为没有context
      };

      // 准备录制
      await this.recorder.prepare(config);
      
      // 开始录制
      await this.recorder.start();
      
      this.isRecording = true;
      hilog.info(0x0000, ScreenRecorderService.TAG, 'Screen recording started');
      
    } catch (error) {
      hilog.error(0x0000, ScreenRecorderService.TAG, 'Start recording failed: %{public}s', JSON.stringify(error));
    }
  }

  /**
   * 停止录制
   */
  private async stopRecording(): Promise<void> {
    if (!this.isRecording || !this.recorder) {
      hilog.warn(0x0000, ScreenRecorderService.TAG, 'Not recording');
      return;
    }

    try {
      await this.recorder.stop();
      await this.recorder.release();
      
      this.recorder = null;
      this.isRecording = false;
      
      hilog.info(0x0000, ScreenRecorderService.TAG, 'Screen recording stopped');
      
    } catch (error) {
      hilog.error(0x0000, ScreenRecorderService.TAG, 'Stop recording failed: %{public}s', JSON.stringify(error));
    }
  }

  /**
   * 暂停录制
   */
  private async pauseRecording(): Promise<void> {
    if (!this.isRecording || !this.recorder) {
      return;
    }

    try {
      await this.recorder.pause();
      hilog.info(0x0000, ScreenRecorderService.TAG, 'Screen recording paused');
    } catch (error) {
      hilog.error(0x0000, ScreenRecorderService.TAG, 'Pause recording failed: %{public}s', JSON.stringify(error));
    }
  }

  /**
   * 恢复录制
   */
  private async resumeRecording(): Promise<void> {
    if (!this.isRecording || !this.recorder) {
      return;
    }

    try {
      await this.recorder.resume();
      hilog.info(0x0000, ScreenRecorderService.TAG, 'Screen recording resumed');
    } catch (error) {
      hilog.error(0x0000, ScreenRecorderService.TAG, 'Resume recording failed: %{public}s', JSON.stringify(error));
    }
  }

  /**
   * 获取录制状态
   */
  getRecordingStatus(): boolean {
    return this.isRecording;
  }
}
