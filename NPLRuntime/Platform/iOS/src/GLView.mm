//
//  GLView.m
//  ios_gl
//
//  Created by 袁全伟 on 2017/11/12.
//  Copyright © 2017年 XRenderAPI. All rights reserved.
//

#import "GLView.h"

@interface GLView ()
{

}
@end


@implementation GLView

+(Class) layerClass
{
    return [CAEAGLLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    

    
    return self;
}



@end
