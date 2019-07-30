#import <UIKit/UIKit.h>
#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "2dengine/GUIEdit.h"

using namespace ParaEngine;

bool ParaEngine::StringHelper::CopyTextToClipboard(const string& text_)
{
   
//    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
//    [pasteboard clearContents];  //必须清空，否则setString会失败。
//    [pasteboard setString: @(text_.c_str()) forType: NSStringPboardType];

    return true;
}

 const char* ParaEngine::StringHelper::GetTextFromClipboard()
 {
//     NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
//     NSArray *types = [pasteboard types];
//     if ([types containsObject:NSPasteboardTypeString]) {
//         NSString *s = [pasteboard stringForType:NSPasteboardTypeString];
//
//         return [s UTF8String];
//     }

     return "";
 }

void ParaEngine::CGUIEditBox::CopyToClipboard()
{
    std::string text;
    StringHelper::UTF16ToUTF8(m_Buffer.GetBuffer(), text);
    
//    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
//    [pasteboard clearContents];  //必须清空，否则setString会失败。
//    [pasteboard setString: @(text.c_str()) forType: NSStringPboardType];
    
    return true;
}

void ParaEngine::CGUIEditBox::PasteFromClipboard()
{
//    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
//    NSArray *types = [pasteboard types];
//    if ([types containsObject:NSPasteboardTypeString]) {
//        NSString *s = [pasteboard stringForType:NSPasteboardTypeString];
//
//        m_Buffer.Clear();
//
//        std::u16string text;
//        StringHelper::UTF8ToUTF16([s UTF8String], text);
//
//        m_Buffer.SetText(text.c_str());
//    }
}
