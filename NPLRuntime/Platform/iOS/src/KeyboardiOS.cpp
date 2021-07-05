//
//  KeyboardiOS.cpp
//  Paracraft
//
//  Created by big on 2021/6/26.
//

#include "KeyboardiOS.h"
#include <unordered_map>
#include "ParaEngine.h"
#include "Framework/InputSystem/VirtualKey.h"
#include "2dengine/GUIRoot.h"

namespace ParaEngine
{
    vector<string> KeyboardiOS::allPressesKey;

    string KeyboardiOS::intToString(int i) {
        char cStr = i;
        char *cStrArr = new char();
        cStrArr[0] = cStr;
        string str = cStrArr;

        return str;
    }

    EVirtualKey KeyboardiOS::toVirtualKey(const string key) {
        static std::unordered_map<string, EVirtualKey> s_keymap;

        if (s_keymap.size() == 0) {
            // [a-z]
            s_keymap[intToString(97)] = EVirtualKey::KEY_A;
            s_keymap[intToString(98)] = EVirtualKey::KEY_B;
            s_keymap[intToString(99)] = EVirtualKey::KEY_C;
            s_keymap[intToString(100)] = EVirtualKey::KEY_D;
            s_keymap[intToString(101)] = EVirtualKey::KEY_E;
            s_keymap[intToString(102)] = EVirtualKey::KEY_F;
            s_keymap[intToString(103)] = EVirtualKey::KEY_G;
            s_keymap[intToString(104)] = EVirtualKey::KEY_H;
            s_keymap[intToString(105)] = EVirtualKey::KEY_I;
            s_keymap[intToString(106)] = EVirtualKey::KEY_J;
            s_keymap[intToString(107)] = EVirtualKey::KEY_K;
            s_keymap[intToString(108)] = EVirtualKey::KEY_L;
            s_keymap[intToString(109)] = EVirtualKey::KEY_M;
            s_keymap[intToString(110)] = EVirtualKey::KEY_N;
            s_keymap[intToString(111)] = EVirtualKey::KEY_O;
            s_keymap[intToString(112)] = EVirtualKey::KEY_P;
            s_keymap[intToString(113)] = EVirtualKey::KEY_Q;
            s_keymap[intToString(114)] = EVirtualKey::KEY_R;
            s_keymap[intToString(115)] = EVirtualKey::KEY_S;
            s_keymap[intToString(116)] = EVirtualKey::KEY_T;
            s_keymap[intToString(117)] = EVirtualKey::KEY_U;
            s_keymap[intToString(118)] = EVirtualKey::KEY_V;
            s_keymap[intToString(119)] = EVirtualKey::KEY_W;
            s_keymap[intToString(120)] = EVirtualKey::KEY_X;
            s_keymap[intToString(121)] = EVirtualKey::KEY_Y;
            s_keymap[intToString(122)] = EVirtualKey::KEY_Z;
            
            // [0-9]
            s_keymap[intToString(48)] = EVirtualKey::KEY_0;
            s_keymap[intToString(49)] = EVirtualKey::KEY_1;
            s_keymap[intToString(50)] = EVirtualKey::KEY_2;
            s_keymap[intToString(51)] = EVirtualKey::KEY_3;
            s_keymap[intToString(52)] = EVirtualKey::KEY_4;
            s_keymap[intToString(53)] = EVirtualKey::KEY_5;
            s_keymap[intToString(54)] = EVirtualKey::KEY_6;
            s_keymap[intToString(55)] = EVirtualKey::KEY_7;
            s_keymap[intToString(56)] = EVirtualKey::KEY_8;
            s_keymap[intToString(57)] = EVirtualKey::KEY_9;
            
            // special key
            s_keymap[intToString(32)] = EVirtualKey::KEY_SPACE;
            s_keymap[intToString(13)] = EVirtualKey::KEY_RETURN;
            s_keymap["UIKeyInputUpArrow"] = EVirtualKey::KEY_UP;
            s_keymap["UIKeyInputDownArrow"] = EVirtualKey::KEY_DOWN;
            s_keymap["UIKeyInputLeftArrow"] = EVirtualKey::KEY_LEFT;
            s_keymap["UIKeyInputRightArrow"] = EVirtualKey::KEY_RIGHT;
            s_keymap["UIKeyInputEscape"] = EVirtualKey::KEY_ESCAPE;
            s_keymap["UIKeyInputPageUp"] = EVirtualKey::KEY_PRIOR;
            s_keymap["UIKeyInputPageDown"] = EVirtualKey::KEY_NEXT;
            s_keymap["UIKeyInputHome"] = EVirtualKey::KEY_HOME;
            s_keymap["UIKeyInputEnd"] = EVirtualKey::KEY_END;
            s_keymap["UIKeyInputF1"] = EVirtualKey::KEY_F1;
            s_keymap["UIKeyInputF2"] = EVirtualKey::KEY_F2;
            s_keymap["UIKeyInputF3"] = EVirtualKey::KEY_F3;
            s_keymap["UIKeyInputF4"] = EVirtualKey::KEY_F4;
            s_keymap["UIKeyInputF5"] = EVirtualKey::KEY_F5;
            s_keymap["UIKeyInputF6"] = EVirtualKey::KEY_F6;
            s_keymap["UIKeyInputF7"] = EVirtualKey::KEY_F7;
            s_keymap["UIKeyInputF8"] = EVirtualKey::KEY_F8;
            s_keymap["UIKeyInputF9"] = EVirtualKey::KEY_F9;
            s_keymap["UIKeyInputF10"] = EVirtualKey::KEY_F10;
            s_keymap["UIKeyInputF11"] = EVirtualKey::KEY_F11;
            s_keymap["UIKeyInputF12"] = EVirtualKey::KEY_F12;
            s_keymap["FlagKeyInputControl"] = EVirtualKey::KEY_LCONTROL;
            s_keymap["FlagKeyInputCommand"] = EVirtualKey::KEY_LCONTROL;
            s_keymap["FlagKeyInputAlt"] = EVirtualKey::KEY_LMENU;
            s_keymap["FlagKeyInputShift"] = EVirtualKey::KEY_LSHIFT;
        }
        
        std::unordered_map<string, EVirtualKey>::iterator ret = s_keymap.find(key);

        if (ret != s_keymap.end()) {
            return ret->second;
        } else {
            return EVirtualKey::KEY_UNKNOWN;
        }
    }

    void KeyboardiOS::OnKeyDown(const string key)
    {
        if (IsKeyPress(key)) {
            return;
        }

        CGUIRoot::GetInstance()->SendKeyDownEvent(toVirtualKey(key));

        allPressesKey.push_back(key);
    }

    void KeyboardiOS::OnKeyUp()
    {
        if (allPressesKey.size() > 0) {
            CGUIRoot::GetInstance()->SendKeyUpEvent(toVirtualKey(allPressesKey.back()));

            allPressesKey.pop_back();
        }
    }

    bool KeyboardiOS::IsKeyPress(const string key)
    {
        for (int i = 0; i < allPressesKey.size(); i++) {
            if (allPressesKey[i] == key) {
                return true;
            }
        }

        return false;
    }
}

