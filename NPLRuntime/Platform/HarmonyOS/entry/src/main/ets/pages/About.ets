@Entry
@Component
struct About {
  @State message: string = '关于帕拉卡'

  build() {
    RelativeContainer() {
      Text(this.message)
        .id('AboutTitle')
        .fontSize(40)
        .fontWeight(FontWeight.Bold)
        .alignRules({
          top: { anchor: '__container__', align: VerticalAlign.Top },
          middle: { anchor: '__container__', align: HorizontalAlign.Center }
        })
        .margin({ top: 100 })

      Text('版本信息：v1.4\n开发团队：塔夫斗科技\n3D虚拟社区学习软件')
        .id('AboutContent')
        .fontSize(16)
        .textAlign(TextAlign.Center)
        .alignRules({
          center: { anchor: '__container__', align: VerticalAlign.Center },
          middle: { anchor: '__container__', align: HorizontalAlign.Center }
        })
    }
    .height('100%')
    .width('100%')
  }
}
