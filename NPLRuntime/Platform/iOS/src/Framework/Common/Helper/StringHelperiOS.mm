#import <UIKit/UIKit.h>
#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "2dengine/GUIEdit.h"

using namespace ParaEngine;

bool ParaEngine::StringHelper::CopyTextToClipboard(const string &text_)
{
    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
    pasteboard.string = @(text_.c_str());

    return true;
}

const char *ParaEngine::StringHelper::GetTextFromClipboard()
{
     UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];

     return [pasteboard.string UTF8String];
}

void ParaEngine::CGUIEditBox::CopyToClipboard()
{
    std::string text;
    StringHelper::UTF16ToUTF8(m_Buffer.GetBuffer(), text);

    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
    pasteboard.string = @(text.c_str());
    
    return true;
}

void ParaEngine::CGUIEditBox::PasteFromClipboard()
{
    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];

    std::u16string text;
    StringHelper::UTF8ToUTF16([pasteboard.string UTF8String], text);

    m_Buffer.SetText(text.c_str());
}
