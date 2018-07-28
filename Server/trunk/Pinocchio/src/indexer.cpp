
#include "indexer.h"

unsigned int interLeaveLookupTable[32768];

class LookupTable
{
public:
    LookupTable()
    {
        for(int i = 0; i < 32768; ++i) {
            interLeaveLookupTable[i] = 0;
            for(int k = 0; k < 15; ++k)
                if(i & (1 << k))
                    interLeaveLookupTable[i] += (1 << (28 - 2 * k));
        }
    }
};

static LookupTable lt;

unsigned int interLeave3LookupTable[1024];

class LookupTable3
{
    public:
        LookupTable3()
        {
            for(int i = 0; i < 1024; ++i) {
                interLeave3LookupTable[i] = 0;
                for(int k = 0; k < 10; ++k)
                    if(i & (1 << k))
                        interLeave3LookupTable[i] += (1 << (27 - 3 * k));
            }
        }
};

static LookupTable3 lt3;
