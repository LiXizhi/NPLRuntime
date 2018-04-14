//
//  GLView.m
//  ios_gl
//
//  Created by 袁全伟 on 2017/11/12.
//  Copyright © 2017年 XRenderAPI. All rights reserved.
//

#import "GLView.h"
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

@interface GLView ()
{
    GLint               backingWidth;
    GLint               backingHeight;
    GLuint              frameBuffer;
    GLuint              colorBuffer;
    GLuint              depthBuffer;
    
    CAEAGLLayer *glLayer;
    EAGLContext *glContext;
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
    
    glLayer = (CAEAGLLayer*)self.layer;
    glLayer.opaque = YES;
    glLayer.drawableProperties = @{
                                       kEAGLDrawablePropertyRetainedBacking :[NSNumber numberWithBool:NO],
                                       kEAGLDrawablePropertyColorFormat : kEAGLColorFormatRGBA8
                                       };

    
    glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:glContext];
   
    
    // frame buffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER,frameBuffer);
    
    // color buffer
    glGenRenderbuffers(1, &colorBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    [glContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:glLayer];
    
    // depth buffer
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
    
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    // check framebuffer status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
         NSLog(@"failed to make complete frame buffer object %x", status);
        exit(1);
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);

    
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    [glContext presentRenderbuffer:colorBuffer];
    
    
    return self;
}


@end
