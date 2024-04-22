//
//  ConsoleWindowController.m
//  Paracraft
//
//  Created by apple on 2018/4/26.
//

#import "ConsoleWindowController.h"

@interface ConsoleWindowController ()

@end

const int SKIP_LINES_COUNT  = 3;
const int MAX_LINE_LEN      = 40960;
const int MAX_LINES_COUNT   = 1024;

@implementation ConsoleWindowController
@synthesize textView;


- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self)
    {
        linesCount = [[NSMutableArray arrayWithCapacity:MAX_LINES_COUNT +1] retain];
    }
    
    return self;
}

- (void)dealloc
{
    [linesCount release];
    [super dealloc];
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (void) trace:(NSString*)msg
{
    if (traceCount >= SKIP_LINES_COUNT && [msg length] > MAX_LINE_LEN)
    {
        msg = [NSString stringWithFormat:@"%@ ...", [msg substringToIndex:MAX_LINE_LEN - 4]];
    }
    traceCount++;
    NSFont *font = [NSFont fontWithName:@"Monaco" size:12.0];
    NSDictionary *attrsDictionary = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
    NSAttributedString *string = [[NSAttributedString alloc] initWithString:msg attributes:attrsDictionary];
    NSNumber *len = [NSNumber numberWithUnsignedInteger:[string length]];
    [linesCount addObject:len];
    
    NSTextStorage *storage = [textView textStorage];
    [storage beginEditing];
    [storage appendAttributedString:string];
    
    if ([linesCount count] >= MAX_LINES_COUNT)
    {
        len = [linesCount objectAtIndex:0];
        [storage deleteCharactersInRange:NSMakeRange(0, [len unsignedIntegerValue])];
        [linesCount removeObjectAtIndex:0];
    }
    
    [storage endEditing];
    [self changeScroll];
}

- (void) changeScroll
{
    bool scroll = [checkScroll state] == NSOnState;
    if (scroll)
    {
        [self.textView scrollRangeToVisible:NSMakeRange(self.textView.string.length, 0)];
    }
}

- (IBAction)onClear:(id)sender
{
    NSTextStorage* storage = [textView textStorage];
    [storage setAttributedString:[[[NSAttributedString alloc] initWithString:@""] autorelease]];
}

- (IBAction)onScrollChange: (id)sender
{
    [self changeScroll];
}

- (IBAction)onTopChange:(id)sender
{
    bool isTop = [topCheckBox state] == NSOnState;
    if (isTop)
    {
        [self.window setLevel:NSFloatingWindowLevel];
    }
    else
    {
          [self.window setLevel:NSNormalWindowLevel];
    }
}

@end
