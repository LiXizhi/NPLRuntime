#ifndef __PPU_ADDRESS_SPACE_H
#define __PPU_ADDRESS_SPACE_H


#ifdef WIN32
//stop those casting warnings until we have a better solution for ppu_address_t / void* / uint64 conversions
#pragma warning (disable: 4311)
#pragma warning (disable: 4312)
#endif //WIN32

#ifdef USE_ADDR64
typedef uint64_t ppu_address_t;
#else
typedef uint32_t ppu_address_t;
#endif

#endif

