@Entry
@Component
struct Index {
  @State message: string = '帕拉卡3D虚拟社区学习软件'

  build() {
    RelativeContainer() {
      Text(this.message)
        .id('IndexHelloWorld')
        .fontSize(50)
        .fontWeight(FontWeight.Bold)
        .alignRules({
          center: { anchor: '__container__', align: VerticalAlign.Center },
          middle: { anchor: '__container__', align: HorizontalAlign.Center }
        })
    }
    .height('100%')
    .width('100%')
  }
}
