#pragma once

#include "cl_memory.h"

#ifndef CL_NON_NATIVE

#define memset              cl_memset
#define memzero             cl_memzero
#define memcpy              cl_memcpy
#define memmove             cl_memmove
#define memcmp              cl_memcmp
#define rmemcmp             cl_rmemcmp
#define memscan             cl_memscan
#endif // CL_NON_NATIVE
