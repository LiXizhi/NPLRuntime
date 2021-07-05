//
//  KeyboardiOS.h
//  Paracraft
//
//  Created by 陈燎 on 2021/6/26.
//

#ifndef KeyboardiOS_h
#define KeyboardiOS_h

#include <stdio.h>
#include "ParaEngine.h"
#include "Framework/InputSystem/VirtualKey.h"

namespace ParaEngine
{
    class KeyboardiOS
    {
    public:
        static string intToString(int i);
        static EVirtualKey toVirtualKey(const string key);
        static void OnChar(const string key);
        static void OnKey(const string key);
    };
}

#endif /* KeyboardiOS_h */
