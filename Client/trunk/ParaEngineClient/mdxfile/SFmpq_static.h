/*

  ShadowFlare MPQ Static Library. (c) ShadowFlare Software 2002

*/

#ifndef SHADOWFLARE_MPQ_LIB_INCLUDED
#define SHADOWFLARE_MPQ_LIB_INCLUDED

#ifndef SFMPQ_STATIC
#define SFMPQ_STATIC
#endif

#include "SFmpqapi.h"
namespace ParaEngine
{
#ifdef __cplusplus
extern "C" {
#endif

// These no longer need to be called.  They are only provided for
// compatibility with older versions of this static library
void LoadSFMpq();
void FreeSFMpq();

#ifdef __cplusplus
};  // extern "C" 
#endif
}
#endif

