#pragma once
/**
Class:	The famous dl malloc written by Doug Lea, where the linux malloc is based on. Please see dl_malloc.h for more information. 
Authors: LiXizhi
Company: ParaEngine
Date:	2009.6.17
Desc: If you want to update to a newer version, be sure to include this header on the first line of dl_malloc.c
I use dl_malloc as a lock-free(thread local) memory allocator (i.e. use with mspace_XXX functions). 
It is mostly for use with NPL runtime state (thread) and main rendering thread (STL containers)

Example:
#include "util/dl_malloc_include.h"

// mspaces as thread-locals. For example:

static __thread mspace tlms = 0;
void*  tlmalloc(size_t bytes) {
if (tlms == 0) tlms = create_mspace(0, 0);
return mspace_malloc(tlms, bytes);
}
void  tlfree(void* mem) { 
mspace_free(tlms, mem); 
}

*/

// we only want dlXXX functions, but keep the system's default malloc for the rest. 
#define USE_DL_PREFIX

// use mspaces, so that we can create thread local heaps. 
#define MSPACES			1

// in fact, we only need thread local heaps. 
#define ONLY_MSPACES	1

// it is assumed that this file will be #included AFTER <malloc.h>, since we want to keep the system's version as well.
#define HAVE_USR_INCLUDE_MALLOC_H

// just need this to make system' version work
#define NO_MALLINFO		1

#ifndef INCLUDE_ONLY_DL_PREDEFINED_OPTIONS
// the original header file
#include "dl_malloc.h"
#endif