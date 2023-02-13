//-----------------------------------------------------------------------------
// Class: ParaTextField.mm
// Authors: big
// CreateDate: 2023.1.11
//-----------------------------------------------------------------------------

#import "ParaTextField.h"
#import "KeyboardiOS.h"

@implementation ParaTextField

- (void)deleteBackward
{
    if (![KeyboardiOSController getIsGuiEdit]) {
        if ([[KeyboardiOSController GetTextField].text length] > 0) {
            [super deleteBackward];
            return;
        }

        auto pGUIIns = ParaEngine::CGUIRoot::GetInstance();
        pGUIIns->SendKeyDownEvent(ParaEngine::EVirtualKey::KEY_BACK);
        pGUIIns->SendKeyUpEvent(ParaEngine::EVirtualKey::KEY_BACK);
    }

    [super deleteBackward];
}

@end
