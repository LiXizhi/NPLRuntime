/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef DEBUGGING_H
#define DEBUGGING_H

#include "mathutils.h"

class Debugging
{
public:
    static ostream &out() { return *outStream; }
    static void PINOCCHIO_API setOutStream(ostream &os) { outStream = &os; }

private:
    static ostream *outStream;
};

#endif //DEBUGGING_H
