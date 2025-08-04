/**
 * ParaEngine EditBox Page for HarmonyOS  
 * 对应Android项目中的ParaEngineEditBoxActivity
 */

import router from '@ohos.router';

@Entry
@Component
struct ParaEngineEditBox {
  @State inputText: string = '';
  @State placeholder: string = '请输入内容';
  @State maxLength: number = 1000;
  @State isMultiline: boolean = true;
  private callbackId: string = '';

  aboutToAppear() {
    // 获取路由参数
    const params = router.getParams() as Record<string, Object>;
    if (params) {
      this.placeholder = params['placeholder'] as string || this.placeholder;
      this.maxLength = params['maxLength'] as number || this.maxLength;
      this.isMultiline = params['multiline'] as boolean || this.isMultiline;
      this.callbackId = params['callbackId'] as string || '';
      this.inputText = params['defaultText'] as string || '';
    }
  }

  build() {
    Column() {
      // 顶部工具栏
      Row() {
        Button('取消')
          .backgroundColor(Color.Transparent)
          .fontColor(Color.White)
          .onClick(() => {
            this.onCancel();
          })

        Blank()

        Text('编辑文本')
          .fontSize(18)
          .fontColor(Color.White)
          .fontWeight(FontWeight.Bold)

        Blank()

        Button('确定')
          .backgroundColor(Color.Blue)
          .fontColor(Color.White)
          .onClick(() => {
            this.onConfirm();
          })
      }
      .width('100%')
      .height(56)
      .padding({ left: 16, right: 16 })
      .backgroundColor('#2F2F2F')

      // 输入框区域
      Column() {
        if (this.isMultiline) {
          TextArea({
            text: this.inputText,
            placeholder: this.placeholder
          })
            .width('100%')
            .height('100%')
            .fontSize(16)
            .backgroundColor(Color.White)
            .borderRadius(8)
            .padding(12)
            .maxLength(this.maxLength)
            .onChange((value: string) => {
              this.inputText = value;
            })
            .onFocus(() => {
              console.info('TextArea focused');
            })
            .onBlur(() => {
              console.info('TextArea blurred');
            })
        } else {
          TextInput({
            text: this.inputText,
            placeholder: this.placeholder
          })
            .width('100%')
            .height(48)
            .fontSize(16)
            .backgroundColor(Color.White)
            .borderRadius(8)
            .padding({ left: 12, right: 12 })
            .maxLength(this.maxLength)
            .onChange((value: string) => {
              this.inputText = value;
            })
            .onSubmit(() => {
              this.onConfirm();
            })
        }

        // 字符计数
        Row() {
          Blank()
          Text(`${this.inputText.length}/${this.maxLength}`)
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .margin({ top: 8 })
      }
      .layoutWeight(1)
      .padding(16)
      .backgroundColor('#F5F5F5')
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F5F5F5')
  }

  /**
   * 确认输入
   */
  private onConfirm(): void {
    console.info('EditBox confirm:', this.inputText);
    
    // 回调到native层
    this.callNativeCallback(this.inputText);
    
    // 关闭页面
    this.closePage();
  }

  /**
   * 取消输入
   */
  private onCancel(): void {
    console.info('EditBox cancel');
    
    // 回调空字符串表示取消
    this.callNativeCallback('');
    
    // 关闭页面
    this.closePage();
  }

  /**
   * 调用native回调
   */
  private callNativeCallback(text: string): void {
    // TODO: 调用native方法传递输入结果
    console.info('Call native callback:', this.callbackId, text);
  }

  /**
   * 关闭页面
   */
  private closePage(): void {
    router.back();
  }

  /**
   * 处理返回键
   */
  onBackPress(): boolean {
    this.onCancel();
    return true; // 拦截返回事件
  }
}
