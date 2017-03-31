#import <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>

#include "fssimplewindow.h"


static int mouseLb=0,mouseMb=0,mouseRb=0;


static int FsNormalKeyCode[256]=
{
	0,                 // 0
	0,                 // 1
	0,                 // 2
	0,                 // 3
	0,                 // 4
	0,                 // 5
	0,                 // 6
	0,                 // 7
	0,                 // 8
	FSKEY_TAB,         // 9
	0,                 // 10
	0,                 // 11
	0,                 // 12
	FSKEY_ENTER,       // 13
	0,                 // 14
	0,                 // 15
	0,                 // 16
	0,                 // 17
	0,                 // 18
	0,                 // 19
	0,                 // 20
	0,                 // 21
	0,                 // 22
	0,                 // 23
	0,                 // 24
	0,                 // 25
	0,                 // 26
	FSKEY_ESC,         // 27
	0,                 // 28
	0,                 // 29
	0,                 // 30
	0,                 // 31
	FSKEY_SPACE,       // 32
	0,                 // 33
	0,                 // 34
	0,                 // 35
	0,                 // 36
	0,                 // 37
	0,                 // 38
	FSKEY_SINGLEQUOTE, // 39
	0,                 // 40
	0,                 // 41
	0,                 // 42
	FSKEY_PLUS,        // 43
	FSKEY_COMMA,       // 44
	FSKEY_MINUS,       // 45
	FSKEY_DOT,         // 46
	FSKEY_SLASH,       // 47
	FSKEY_0,           // 48
	FSKEY_1,           // 49
	FSKEY_2,           // 50
	FSKEY_3,           // 51
	FSKEY_4,           // 52
	FSKEY_5,           // 53
	FSKEY_6,           // 54
	FSKEY_7,           // 55
	FSKEY_8,           // 56
	FSKEY_9,           // 57
	FSKEY_SEMICOLON,   // 58
	0,                 // 59
	0,                 // 60
	0,                 // 61
	0,                 // 62
	0,                 // 63
	0,                 // 64
	FSKEY_A,           // 65
	FSKEY_B,           // 66
	FSKEY_C,           // 67
	FSKEY_D,           // 68
	FSKEY_E,           // 69
	FSKEY_F,           // 70
	FSKEY_G,           // 71
	FSKEY_H,           // 72
	FSKEY_I,           // 73
	FSKEY_J,           // 74
	FSKEY_K,           // 75
	FSKEY_L,           // 76
	FSKEY_M,           // 77
	FSKEY_N,           // 78
	FSKEY_O,           // 79
	FSKEY_P,           // 80
	FSKEY_Q,           // 81
	FSKEY_R,           // 82
	FSKEY_S,           // 83
	FSKEY_T,           // 84
	FSKEY_U,           // 85
	FSKEY_V,           // 86
	FSKEY_W,           // 87
	FSKEY_X,           // 88
	FSKEY_Y,           // 89
	FSKEY_Z,           // 90
	FSKEY_LBRACKET,    // 91
	FSKEY_BACKSLASH,   // 92
	FSKEY_RBRACKET,    // 93
	0,                 // 94
	0,                 // 95
	0,                 // 96
	FSKEY_A,           // 97
	FSKEY_B,           // 98
	FSKEY_C,           // 99
	FSKEY_D,           // 100
	FSKEY_E,           // 101
	FSKEY_F,           // 102
	FSKEY_G,           // 103
	FSKEY_H,           // 104
	FSKEY_I,           // 105
	FSKEY_J,           // 106
	FSKEY_K,           // 107
	FSKEY_L,           // 108
	FSKEY_M,           // 109
	FSKEY_N,           // 110
	FSKEY_O,           // 111
	FSKEY_P,           // 112
	FSKEY_Q,           // 113
	FSKEY_R,           // 114
	FSKEY_S,           // 115
	FSKEY_T,           // 116
	FSKEY_U,           // 117
	FSKEY_V,           // 118
	FSKEY_W,           // 119
	FSKEY_X,           // 120
	FSKEY_Y,           // 121
	FSKEY_Z,           // 122
	FSKEY_LBRACKET,    // 123
	0,                 // 124
	FSKEY_RBRACKET,    // 125
	FSKEY_TILDA,       // 126
	FSKEY_BS,          // 127
	0,                 // 128
	0,                 // 129
	0,                 // 130
	0,                 // 131
	0,                 // 132
	0,                 // 133
	0,                 // 134
	0,                 // 135
	0,                 // 136
	0,                 // 137
	0,                 // 138
	0,                 // 139
	0,                 // 140
	0,                 // 141
	0,                 // 142
	0,                 // 143
	0,                 // 144
	0,                 // 145
	0,                 // 146
	0,                 // 147
	0,                 // 148
	0,                 // 149
	0,                 // 150
	0,                 // 151
	0,                 // 152
	0,                 // 153
	0,                 // 154
	0,                 // 155
	0,                 // 156
	0,                 // 157
	0,                 // 158
	0,                 // 159
	0,                 // 160
	0,                 // 161
	0,                 // 162
	0,                 // 163
	0,                 // 164
	0,                 // 165
	0,                 // 166
	0,                 // 167
	0,                 // 168
	0,                 // 169
	0,                 // 170
	0,                 // 171
	0,                 // 172
	0,                 // 173
	0,                 // 174
	0,                 // 175
	0,                 // 176
	0,                 // 177
	0,                 // 178
	0,                 // 179
	0,                 // 180
	0,                 // 181
	0,                 // 182
	0,                 // 183
	0,                 // 184
	0,                 // 185
	0,                 // 186
	0,                 // 187
	0,                 // 188
	0,                 // 189
	0,                 // 190
	0,                 // 191
	0,                 // 192
	0,                 // 193
	0,                 // 194
	0,                 // 195
	0,                 // 196
	0,                 // 197
	0,                 // 198
	0,                 // 199
	0,                 // 200
	0,                 // 201
	0,                 // 202
	0,                 // 203
	0,                 // 204
	0,                 // 205
	0,                 // 206
	0,                 // 207
	0,                 // 208
	0,                 // 209
	0,                 // 210
	0,                 // 211
	0,                 // 212
	0,                 // 213
	0,                 // 214
	0,                 // 215
	0,                 // 216
	0,                 // 217
	0,                 // 218
	0,                 // 219
	0,                 // 220
	0,                 // 221
	0,                 // 222
	0,                 // 223
	0,                 // 224
	0,                 // 225
	0,                 // 226
	0,                 // 227
	0,                 // 228
	0,                 // 229
	0,                 // 230
	0,                 // 231
	0,                 // 232
	0,                 // 233
	0,                 // 234
	0,                 // 235
	0,                 // 236
	0,                 // 237
	0,                 // 238
	0,                 // 239
	0,                 // 240
	0,                 // 241
	0,                 // 242
	0,                 // 243
	0,                 // 244
	0,                 // 245
	0,                 // 246
	0,                 // 247
	0,                 // 248
	0,                 // 249
	0,                 // 250
	0,                 // 251
	0,                 // 252
	0,                 // 253
	0,                 // 254
	0                  // 255
};

static int FsSpecialKeyCode[256]=
{
	FSKEY_UP,           // 0
	FSKEY_DOWN,         // 1
	FSKEY_LEFT,         // 2
	FSKEY_RIGHT,        // 3
	FSKEY_F1,           // 4
	FSKEY_F2,           // 5
	FSKEY_F3,           // 6
	FSKEY_F4,           // 7
	FSKEY_F5,           // 8
	FSKEY_F6,           // 9
	FSKEY_F7,           // 10
	FSKEY_F8,           // 11
	FSKEY_F9,           // 12
	FSKEY_F10,          // 13
	FSKEY_F11,          // 14
	FSKEY_F12,          // 15
	FSKEY_F1,           // 16 0x10
	FSKEY_F2,           // 17
	FSKEY_F3,           // 18
	FSKEY_F4,           // 19
	FSKEY_F5,           // 20
	FSKEY_F6,           // 21
	FSKEY_F7,           // 22
	FSKEY_F8,           // 23
	FSKEY_F9,           // 24
	FSKEY_F10,          // 25
	FSKEY_F11,          // 26
	FSKEY_F12,          // 27
	FSKEY_F1,           // 28
	FSKEY_F2,           // 29
	FSKEY_F3,           // 30
	FSKEY_F4,           // 31
	FSKEY_F5,           // 32 0x20
	FSKEY_F6,           // 33
	FSKEY_F7,           // 34
	FSKEY_F8,           // 35
	FSKEY_F9,           // 36
	FSKEY_F10,          // 37
	FSKEY_F11,          // 38
	FSKEY_INS,          // 39
	FSKEY_DEL,          // 40
	FSKEY_HOME,         // 41
	0,                  // 42 Begin
	FSKEY_END,          // 43
	FSKEY_PAGEUP,       // 44
	FSKEY_PAGEDOWN,     // 45
	FSKEY_PRINTSCRN,    // 46
	FSKEY_SCROLLLOCK,   // 47
	FSKEY_PAUSEBREAK,   // 48 0x30
	0,                  // 49
	FSKEY_PAUSEBREAK,   // 50
	0,                  // 51 Reset
	0,                  // 52 Stop
	0,                  // 53 Menu
	0,                  // 54 User
	0,                  // 55 System
	FSKEY_PRINTSCRN,    // 56
	0,                  // 57 Clear line
	0,                  // 58 Clear display
	0,                  // 59 Insert line
	0,                  // 60 Delete line
	FSKEY_INS,          // 61
	FSKEY_DEL,          // 62
	FSKEY_PAGEUP,       // 63
	FSKEY_PAGEDOWN,     // 64
	0,                  // 65 Select
	0,                  // 66 Execute
	0,                  // 67 Undo
	0,                  // 68 Redo
	0,                  // 69 Find
	0,                  // 70 Help
	0,                  // 71 Mode Switch
	0,                  // 72
	0,                  // 73
	0,                  // 74
	0,                  // 75
	0,                  // 76
	0,                  // 77
	0,                  // 78
	0,                  // 79
	0,                  // 80
	0,                  // 81
	0,                  // 82
	0,                  // 83
	0,                  // 84
	0,                  // 85
	0,                  // 86
	0,                  // 87
	0,                  // 88
	0,                  // 89
	0,                  // 90
	0,                  // 91
	0,                  // 92
	0,                  // 93
	0,                  // 94
	0,                  // 95
	0,                  // 96
	0,                  // 97
	0,                  // 98
	0,                  // 99
	0,                  // 100
	0,                  // 101
	0,                  // 102
	0,                  // 103
	0,                  // 104
	0,                  // 105
	0,                  // 106
	0,                  // 107
	0,                  // 108
	0,                  // 109
	0,                  // 110
	0,                  // 111
	0,                  // 112
	0,                  // 113
	0,                  // 114
	0,                  // 115
	0,                  // 116
	0,                  // 117
	0,                  // 118
	0,                  // 119
	0,                  // 120
	0,                  // 121
	0,                  // 122
	0,                  // 123
	0,                  // 124
	0,                  // 125
	0,                  // 126
	0,                  // 127
	0,                  // 128
	0,                  // 129
	0,                  // 130
	0,                  // 131
	0,                  // 132
	0,                  // 133
	0,                  // 134
	0,                  // 135
	0,                  // 136
	0,                  // 137
	0,                  // 138
	0,                  // 139
	0,                  // 140
	0,                  // 141
	0,                  // 142
	0,                  // 143
	0,                  // 144
	0,                  // 145
	0,                  // 146
	0,                  // 147
	0,                  // 148
	0,                  // 149
	0,                  // 150
	0,                  // 151
	0,                  // 152
	0,                  // 153
	0,                  // 154
	0,                  // 155
	0,                  // 156
	0,                  // 157
	0,                  // 158
	0,                  // 159
	0,                  // 160
	0,                  // 161
	0,                  // 162
	0,                  // 163
	0,                  // 164
	0,                  // 165
	0,                  // 166
	0,                  // 167
	0,                  // 168
	0,                  // 169
	0,                  // 170
	0,                  // 171
	0,                  // 172
	0,                  // 173
	0,                  // 174
	0,                  // 175
	0,                  // 176
	0,                  // 177
	0,                  // 178
	0,                  // 179
	0,                  // 180
	0,                  // 181
	0,                  // 182
	0,                  // 183
	0,                  // 184
	0,                  // 185
	0,                  // 186
	0,                  // 187
	0,                  // 188
	0,                  // 189
	0,                  // 190
	0,                  // 191
	0,                  // 192
	0,                  // 193
	0,                  // 194
	0,                  // 195
	0,                  // 196
	0,                  // 197
	0,                  // 198
	0,                  // 199
	0,                  // 200
	0,                  // 201
	0,                  // 202
	0,                  // 203
	0,                  // 204
	0,                  // 205
	0,                  // 206
	0,                  // 207
	0,                  // 208
	0,                  // 209
	0,                  // 210
	0,                  // 211
	0,                  // 212
	0,                  // 213
	0,                  // 214
	0,                  // 215
	0,                  // 216
	0,                  // 217
	0,                  // 218
	0,                  // 219
	0,                  // 220
	0,                  // 221
	0,                  // 222
	0,                  // 223
	0,                  // 224
	0,                  // 225
	0,                  // 226
	0,                  // 227
	0,                  // 228
	0,                  // 229
	0,                  // 230
	0,                  // 231
	0,                  // 232
	0,                  // 233
	0,                  // 234
	0,                  // 235
	0,                  // 236
	0,                  // 237
	0,                  // 238
	0,                  // 239
	0,                  // 240
	0,                  // 241
	0,                  // 242
	0,                  // 243
	0,                  // 244
	0,                  // 245
	0,                  // 246
	0,                  // 247
	0,                  // 248
	0,                  // 249
	0,                  // 250
	0,                  // 251
	0,                  // 252
	0,                  // 253
	0,                  // 254
	0                   // 255
};

static int YsMacUnicodeToFsKeyCode(int uni)
{
	if(0<=uni && uni<256)
	{
		return FsNormalKeyCode[uni];
	}
	else if(0xf700<=uni && uni<0xf800)
	{
		return FsSpecialKeyCode[uni-0xf700];
	}
	return 0;
}


struct FsMouseEventLog
{
	int eventType;
	int lb,mb,rb;
	int mx,my;
};


static int fsKeyIsDown[FSKEY_NUM_KEYCODE]={0};

#define NKEYBUF 256
static int nKeyBufUsed=0;
static int keyBuffer[NKEYBUF];

static int nCharBufUsed=0;
static int charBuffer[NKEYBUF];

static int nMosBufUsed=0;
static struct FsMouseEventLog mosBuffer[NKEYBUF];

static int exposure=0;


@interface YsMacDelegate : NSObject /* < NSApplicationDelegate > */
/* Example: Fire has the same problem no explanation */
{
}
/* - (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication; */
@end

@implementation YsMacDelegate
- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
	return YES;
}
@end



extern void FsOnClosed();

@interface YsOpenGLWindow : NSWindow
{
}

@end

@implementation YsOpenGLWindow
- (id) initWithContentRect: (NSRect)rect styleMask:(NSUInteger)wndStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferFlg
{
	[super initWithContentRect:rect styleMask:wndStyle backing:bufferingType defer:deferFlg];

	[[NSNotificationCenter defaultCenter]
		addObserver:self
		selector:@selector(windowDidResize:)
		name:NSWindowDidResizeNotification
		object:self];

	[[NSNotificationCenter defaultCenter]
	  addObserver:self
	  selector:@selector(windowWillClose:)
	  name:NSWindowWillCloseNotification
	  object:self];

	[self setAcceptsMouseMovedEvents:YES];

	printf("%s\n",__FUNCTION__);
	return self;
}

- (void) windowDidResize: (NSNotification *)notification
{
}

- (void) windowWillClose: (NSNotification *)notification
{
	[NSApp terminate:nil];	// This can also be exit(0);

	FsOnClosed();
}

@end


@interface YsOpenGLView : NSOpenGLView
{
}
- (void) drawRect: (NSRect) bounds;
@end

@implementation YsOpenGLView
-(void) drawRect: (NSRect) bounds
{
	printf("%s\n",__FUNCTION__);
	exposure=1;
}

-(void) prepareOpenGL
{
	printf("%s\n",__FUNCTION__);
}

-(NSMenu *)menuForEvent: (NSEvent *)theEvent
{
	printf("%s\n",__FUNCTION__);
	return [NSView defaultMenu];
}

- (void) flagsChanged: (NSEvent *)theEvent
{
	unsigned int flags;
	flags=[theEvent modifierFlags];

	if(flags&NSAlphaShiftKeyMask) // Caps
	{
		if(fsKeyIsDown[FSKEY_CAPSLOCK]==0 && nKeyBufUsed<NKEYBUF)
		{
			keyBuffer[nKeyBufUsed++]=FSKEY_CAPSLOCK;
		}
		fsKeyIsDown[FSKEY_CAPSLOCK]=1;
	}
	else
	{
		fsKeyIsDown[FSKEY_CAPSLOCK]=0;
	}

	if(flags&NSShiftKeyMask)
	{
		if(fsKeyIsDown[FSKEY_SHIFT]==0 && nKeyBufUsed<NKEYBUF)
		{
			keyBuffer[nKeyBufUsed++]=FSKEY_SHIFT;
		}
		fsKeyIsDown[FSKEY_SHIFT]=1;
	}
	else
	{
		fsKeyIsDown[FSKEY_SHIFT]=0;
	}

	if(flags&NSControlKeyMask)
	{
		if(fsKeyIsDown[FSKEY_CTRL]==0 && nKeyBufUsed<NKEYBUF)
		{
			keyBuffer[nKeyBufUsed++]=FSKEY_CTRL;
		}
		fsKeyIsDown[FSKEY_CTRL]=1;
	}
	else
	{
		fsKeyIsDown[FSKEY_CTRL]=0;
	}

	if((flags&NSAlternateKeyMask) || (flags&NSCommandKeyMask))
	{
		if(fsKeyIsDown[FSKEY_ALT]==0 && nKeyBufUsed<NKEYBUF)
		{
			keyBuffer[nKeyBufUsed++]=FSKEY_ALT;
		}
		fsKeyIsDown[FSKEY_ALT]=1;
	}
	else
	{
		fsKeyIsDown[FSKEY_ALT]=0;
	}

	// Other possible key masks
	// NSNumericPadKeyMask
	// NSHelpKeyMask
	// NSFunctionKeyMask
	// NSDeviceIndependentModifierFlagsMask
}

- (void) keyDown:(NSEvent *)theEvent
{
	unsigned int flags;
	flags=[theEvent modifierFlags];

	NSString *chrs,*chrsNoMod;
	chrs=[theEvent characters];
	if(0==(flags & NSCommandKeyMask) && [chrs length]>0)
	{
		int unicode;
		unicode=[chrs characterAtIndex:0];

		if(32<=unicode && unicode<128 && nCharBufUsed<NKEYBUF)
		{
			charBuffer[nCharBufUsed++]=unicode;
		}
	}

	chrsNoMod=[theEvent charactersIgnoringModifiers];
	if([chrsNoMod length]>0)
	{
		int unicode,fskey;
		unicode=[chrsNoMod characterAtIndex:0];
		fskey=YsMacUnicodeToFsKeyCode(unicode);

		if(fskey!=0)
		{
			fsKeyIsDown[fskey]=1;

			if(nKeyBufUsed<NKEYBUF)
			{
				keyBuffer[nKeyBufUsed++]=fskey;
			}
		}
	}
}

- (void) keyUp:(NSEvent *)theEvent
{
	NSString *chrs,*chrsNoMod;
	chrs=[theEvent characters];
	if([chrs length]>0)
	{
		int unicode;
		unicode=[chrs characterAtIndex:0];
	}

	chrsNoMod=[theEvent charactersIgnoringModifiers];
	if([chrsNoMod length]>0)
	{
	  int unicode,fskey;
		unicode=[chrsNoMod characterAtIndex:0];
		fskey=YsMacUnicodeToFsKeyCode(unicode);

		if(fskey!=0)
		{
			fsKeyIsDown[fskey]=0;
		}
	}
}

- (void) mouseMoved:(NSEvent *)theEvent
{
	if(0<nMosBufUsed &&
	   FSMOUSEEVENT_MOVE==mosBuffer[nMosBufUsed-1].eventType)
	{
	  NSRect rect;
	  rect=[self frame];

		mosBuffer[nMosBufUsed-1].mx=(int)[theEvent locationInWindow].x;
		mosBuffer[nMosBufUsed-1].my=rect.size.height-1-(int)[theEvent locationInWindow].y;
	}
	else if(NKEYBUF>nMosBufUsed)
	{
	  NSRect rect;
	  rect=[self frame];

		mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_MOVE;
		mosBuffer[nMosBufUsed].mx=(int)[theEvent locationInWindow].x;
		mosBuffer[nMosBufUsed].my=rect.size.height-1-(int)[theEvent locationInWindow].y;
		mosBuffer[nMosBufUsed].lb=mouseLb;
		mosBuffer[nMosBufUsed].mb=mouseMb;
		mosBuffer[nMosBufUsed].rb=mouseRb;
		nMosBufUsed++;
	}
}

- (void) mouseDragged:(NSEvent *)theEvent
{
  [self mouseMoved:theEvent];
}

- (void) rightMouseDragged:(NSEvent *)theEvent
{
  [self mouseMoved:theEvent];
}

- (void) otherMouseDragged:(NSEvent *)theEvent
{
  [self mouseMoved:theEvent];
}

- (void) mouseDown:(NSEvent *)theEvent
{
	mouseLb=1;

	if(NKEYBUF>nMosBufUsed)
	{
	  NSRect rect;
	  rect=[self frame];

		mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_LBUTTONDOWN;
		mosBuffer[nMosBufUsed].mx=(int)[theEvent locationInWindow].x;
		mosBuffer[nMosBufUsed].my=rect.size.height-1-(int)[theEvent locationInWindow].y;
		mosBuffer[nMosBufUsed].lb=mouseLb;
		mosBuffer[nMosBufUsed].mb=mouseMb;
		mosBuffer[nMosBufUsed].rb=mouseRb;
		nMosBufUsed++;
	}
}

- (void) mouseUp:(NSEvent *)theEvent
{
	mouseLb=0;

	if(NKEYBUF>nMosBufUsed)
	{
	  NSRect rect;
	  rect=[self frame];

		mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_LBUTTONUP;
		mosBuffer[nMosBufUsed].mx=(int)[theEvent locationInWindow].x;
		mosBuffer[nMosBufUsed].my=rect.size.height-1-(int)[theEvent locationInWindow].y;
		mosBuffer[nMosBufUsed].lb=mouseLb;
		mosBuffer[nMosBufUsed].mb=mouseMb;
		mosBuffer[nMosBufUsed].rb=mouseRb;
		nMosBufUsed++;
	}
}

- (void) rightMouseDown:(NSEvent *)theEvent
{
	mouseRb=1;

	if(NKEYBUF>nMosBufUsed)
	{
	  NSRect rect;
	  rect=[self frame];

		mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_RBUTTONDOWN;
		mosBuffer[nMosBufUsed].mx=(int)[theEvent locationInWindow].x;
		mosBuffer[nMosBufUsed].my=rect.size.height-1-(int)[theEvent locationInWindow].y;
		mosBuffer[nMosBufUsed].lb=mouseLb;
		mosBuffer[nMosBufUsed].mb=mouseMb;
		mosBuffer[nMosBufUsed].rb=mouseRb;
		nMosBufUsed++;
	}
}

- (void) rightMouseUp:(NSEvent *)theEvent
{
	mouseRb=0;

	if(NKEYBUF>nMosBufUsed)
	{
	  NSRect rect;
	  rect=[self frame];

		mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_RBUTTONUP;
		mosBuffer[nMosBufUsed].mx=(int)[theEvent locationInWindow].x;
		mosBuffer[nMosBufUsed].my=rect.size.height-1-(int)[theEvent locationInWindow].y;
		mosBuffer[nMosBufUsed].lb=mouseLb;
		mosBuffer[nMosBufUsed].mb=mouseMb;
		mosBuffer[nMosBufUsed].rb=mouseRb;
		nMosBufUsed++;
	}
}

- (void) otherMouseDown:(NSEvent *)theEvent
{
	mouseMb=1;

	if(NKEYBUF>nMosBufUsed)
	{
	  NSRect rect;
	  rect=[self frame];

		mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_MBUTTONDOWN;
		mosBuffer[nMosBufUsed].mx=(int)[theEvent locationInWindow].x;
		mosBuffer[nMosBufUsed].my=rect.size.height-1-(int)[theEvent locationInWindow].y;
		mosBuffer[nMosBufUsed].lb=mouseLb;
		mosBuffer[nMosBufUsed].mb=mouseMb;
		mosBuffer[nMosBufUsed].rb=mouseRb;
		nMosBufUsed++;
	}
}

- (void) otherMouseUp:(NSEvent *)theEvent
{
	mouseMb=0;

	if(NKEYBUF>nMosBufUsed)
	{
	  NSRect rect;
	  rect=[self frame];

		mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_MBUTTONUP;
		mosBuffer[nMosBufUsed].mx=(int)[theEvent locationInWindow].x;
		mosBuffer[nMosBufUsed].my=rect.size.height-1-(int)[theEvent locationInWindow].y;
		mosBuffer[nMosBufUsed].lb=mouseLb;
		mosBuffer[nMosBufUsed].mb=mouseMb;
		mosBuffer[nMosBufUsed].rb=mouseRb;
		nMosBufUsed++;
	}
}

@end



void YsAddMenu(void)
{
 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	NSMenu *mainMenu;

	mainMenu=[NSMenu alloc];
	[mainMenu initWithTitle:@"Minimum"];

	NSMenuItem *fileMenu;
	fileMenu=[[NSMenuItem alloc] initWithTitle:@"File" action:NULL keyEquivalent:[NSString string]];
	[mainMenu addItem:fileMenu];

	NSMenu *fileSubMenu;
	fileSubMenu=[[NSMenu alloc] initWithTitle:@"File"];
	[fileMenu setSubmenu:fileSubMenu];

	NSMenuItem *fileMenu_Quit;
	fileMenu_Quit=[[NSMenuItem alloc] initWithTitle:@"Quit"  action:@selector(terminate:) keyEquivalent:@"q"];
	[fileMenu_Quit setTarget:NSApp];
	[fileSubMenu addItem:fileMenu_Quit];

	[NSApp setMainMenu:mainMenu];

	[pool release];
}

void YsTestApplicationPath(void)
{
 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	char cwd[256];
	getcwd(cwd,255);
	printf("CWD(Initial): %s\n",cwd);

	NSString *path;
	path=[[NSBundle mainBundle] bundlePath];
	printf("BundlePath:%s\n",[path UTF8String]);

	[[NSFileManager defaultManager] changeCurrentDirectoryPath:path];

	getcwd(cwd,255);
	printf("CWD(Changed): %s\n",cwd);

	[pool release];
}




static YsOpenGLWindow *ysWnd=nil;
static YsOpenGLView *ysView=nil;

void FsOpenWindowC(int x0,int y0,int wid,int hei,int useDoubleBuffer)
{
 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];


	[NSApplication sharedApplication];
	[NSBundle loadNibNamed:@"MainMenu" owner:NSApp];

	YsMacDelegate *delegate;
	delegate=[YsMacDelegate alloc];
	[delegate init];
	[NSApp setDelegate: delegate];

	[NSApp finishLaunching];



	NSRect contRect;
	contRect=NSMakeRect(x0,y0,wid,hei);

	unsigned int winStyle=
	  NSTitledWindowMask|
	  NSClosableWindowMask|
	  NSMiniaturizableWindowMask|
	  NSResizableWindowMask;

	ysWnd=[YsOpenGLWindow alloc];
	[ysWnd
		initWithContentRect:contRect
		styleMask:winStyle
		backing:NSBackingStoreBuffered
		defer:NO];

	NSOpenGLPixelFormat *format;
	NSOpenGLPixelFormatAttribute formatAttrib[]=
	{
		NSOpenGLPFAWindow,
		NSOpenGLPFADepthSize,(NSOpenGLPixelFormatAttribute)32,
		NSOpenGLPFADoubleBuffer,
		0
	};

	if(useDoubleBuffer==0)
	{
		formatAttrib[3]=0;
	}

	format=[NSOpenGLPixelFormat alloc];
	[format initWithAttributes: formatAttrib];

	ysView=[YsOpenGLView alloc];
	contRect=NSMakeRect(0,0,800,600);
	[ysView
		initWithFrame:contRect
		pixelFormat:format];

	[ysWnd setContentView:ysView];
	[ysWnd makeFirstResponder:ysView];

	[ysWnd makeKeyAndOrderFront:nil];
	[ysWnd makeMainWindow];

	[NSApp activateIgnoringOtherApps:YES];

	YsAddMenu();

	[pool release];


	int i;
	for(i=0; i<FSKEY_NUM_KEYCODE; i++)
	{
		fsKeyIsDown[i]=0;
	}


    glClearColor(1.0F,1.0F,1.0F,0.0F);
    glClearDepth(1.0F);
	glDisable(GL_DEPTH_TEST);

	glViewport(0,0,wid,hei);

    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,(float)wid-1,(float)hei-1,0,-1,1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_FLAT);
	glPointSize(1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor3ub(0,0,0);
}

void FsGetWindowSizeC(int *wid,int *hei)
{
	NSRect rect;
	rect=[ysView frame];
	*wid=rect.size.width;
	*hei=rect.size.height;
}

void FsPollDeviceC(void)
{
 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	while(1)
	{
	 	[pool release];
	 	pool=[[NSAutoreleasePool alloc] init];

		NSEvent *event;
		event=[NSApp
			   nextEventMatchingMask:NSAnyEventMask
			   untilDate: [NSDate distantPast]
			   inMode: NSDefaultRunLoopMode
			   dequeue:YES];
		if([event type]==NSRightMouseDown)
		  {
		    printf("R mouse down event\n");
		  }
		if(event!=nil)
		{
			[NSApp sendEvent:event];
			[NSApp updateWindows];
		}
		else
		{
			break;
		}
	}

	[pool release];
}

void FsSleepC(int ms)
{
	if(ms>0)
	{
		double sec;
		sec=(double)ms/1000.0;
		[NSThread sleepForTimeInterval:sec];
	}
}

int FsPassedTimeC(void)
{
	int ms;

 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	static NSTimeInterval last=0.0;
	NSTimeInterval now;

	now=[[NSDate date] timeIntervalSince1970];

	NSTimeInterval passed;
	passed=now-last;
	ms=(int)(1000.0*passed);

	if(ms<0)
	{
		ms=1;
	}
	last=now;

	[pool release];

	return ms;
}

void FsMouseC(int *lb,int *mb,int *rb,int *mx,int *my)
{
	*lb=mouseLb;
	*mb=mouseMb;
	*rb=mouseRb;

	NSPoint loc;
	loc=[NSEvent mouseLocation];
	loc=[ysWnd convertScreenToBase:loc];
	loc=[ysView convertPointFromBase:loc];

	NSRect rect;
	rect=[ysView frame];
	*mx=loc.x;
	*my=rect.size.height-1-loc.y;
}

int FsGetMouseEventC(int *lb,int *mb,int *rb,int *mx,int *my)
{
	if(0<nMosBufUsed)
	{
		const int eventType=mosBuffer[0].eventType;
		*lb=mosBuffer[0].lb;
		*mb=mosBuffer[0].mb;
		*rb=mosBuffer[0].rb;
		*mx=mosBuffer[0].mx;
		*my=mosBuffer[0].my;

		int i;
		for(i=0; i<nMosBufUsed-1; i++)
		{
			mosBuffer[i]=mosBuffer[i+1];
		}

		nMosBufUsed--;
		return eventType;
	}
	else
	{
		FsMouseC(lb,mb,rb,mx,my);
		return FSMOUSEEVENT_NONE;
	}
}

void FsSwapBufferC(void)
{
	[[ysView openGLContext] flushBuffer];
}

int FsInkeyC(void)
{
	if(nKeyBufUsed>0)
	{
		int i,fskey;
		fskey=keyBuffer[0];
		nKeyBufUsed--;
		for(i=0; i<nKeyBufUsed; i++)
		{
			keyBuffer[i]=keyBuffer[i+1];
		}
		return fskey;
	}
	return 0;
}

int FsInkeyCharC(void)
{
	if(nCharBufUsed>0)
	{
		int i,c;
		c=charBuffer[0];
		nCharBufUsed--;
		for(i=0; i<nCharBufUsed; i++)
		{
			charBuffer[i]=charBuffer[i+1];
		}
		return c;
	}
	return 0;
}

int FsKeyStateC(int fsKeyCode)
{
	if(0<=fsKeyCode && fsKeyCode<FSKEY_NUM_KEYCODE)
	{
		return fsKeyIsDown[fsKeyCode];
	}
	return 0;
}

void FsChangeToProgramDirC(void)
{
	NSString *path;
	path=[[NSBundle mainBundle] bundlePath];
	printf("BundlePath:%s\n",[path UTF8String]);

	[[NSFileManager defaultManager] changeCurrentDirectoryPath:path];
}

int FsCheckExposureC(void)
{
	int ret;
	ret=exposure;
	exposure=0;
	return ret;
}


/* int main(int argc, char *argv[])
{
	YsTestApplicationPath();

	YsOpenWindow();

	printf("Going into the event loop\n");

	double angle;
	angle=0.0;
	while(1)
	{
		YsPollEvent();

		DrawTriangle(angle);
		angle=angle+0.05;

		YsSleep(20);
	}

	return 0;
	} */
