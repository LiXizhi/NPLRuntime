//
//  KeyboardiOS.cpp
//  Paracraft
//
//  Created by 陈燎 on 2021/6/26.
//

#include "KeyboardiOS.h"
#include <unordered_map>
#include "2dengine/GUIRoot.h"

namespace ParaEngine
{
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
            s_keymap[intToString(32)] = EVirtualKey::KEY_SPACE;
            s_keymap["w"] = EVirtualKey::KEY_W;

//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
//            s_keymap[intToString(0)] = EVirtualKey::KEY_A;
            
//            for (int i = 0; i <= 127; i++) {
//                char cStr = i;
//                char *cStrArr = new char();
//                cStrArr[0] = cStr;
//                OUTPUT_LOG("test code to string: %s ", cStrArr);
//                string str = cStrArr;
//                s_keymap[str] = EVirtualKey::KEY_A;
//            }
        }
        
        std::unordered_map<string, EVirtualKey>::iterator ret = s_keymap.find(key);

        if (ret != s_keymap.end()) {
            OUTPUT_LOG("press!!!! -----> %s ", key.c_str());
            return ret->second;
        } else {
            return EVirtualKey::KEY_UNKNOWN;
        }
    }

    void KeyboardiOS::OnKey(const string key)
    {
        //CGUIRoot::GetInstance()->SendKeyDownEvent(toVirtualKey(key));
//        CGUIRoot::GetInstance()->SendKeyUpEvent(toVirtualKey(key));
        OUTPUT_LOG("from on key ----> %s \n", key.c_str());
    }
}

