#pragma once

C_HEADER_START
#include "data_type.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x)   (x);
#endif // UNREFERENCED_PARAMETER

#ifndef NOTHING
#define NOTHING                     ;
#endif // NOTHING

#define MAX_PATH                    260

#define KB_SIZE                     1024ULL
#define MB_SIZE                     (1024*KB_SIZE)
#define GB_SIZE                     (1024*MB_SIZE)
#define TB_SIZE                     (1024*GB_SIZE)

#define PAGE_SIZE                   0x1000

#ifndef PAGE_SHIFT
#define PAGE_SHIFT                  12
#endif // PAGE_SHIFT

#define SHADOW_STACK_SIZE                   0x20

#define BITS_PER_BYTE                       8
#define BITS_FOR_STRUCTURE(x)               (BITS_PER_BYTE*sizeof(x))
#define CREATE_BIT_MASK_FOR_N_BITS(n)       (((n) >= 64) ? MAX_QWORD : (((1ULL << (n)) - 1)))

// The difference between these 2 is the following one:
// IsFlagOn succeeds if at LEAST a flag is set
// IsBooleanFlagOn succeeds if ALL flags are set
#define IsFlagOn(x,f)                       (0!=((x)&(f)))
#define IsBooleanFlagOn(x,f)                ((f)==((x)&(f)))

#define BooleanToInteger(x)                 (!!(x))

// Checks if a bit is set
#define IsBitSet(x,b)                       ((1<<(b))==((x)&(1<<(b))))

// Sets a bit
#define SetBit(x,b)                         ((x)|=((x)|(1<<(b))))

// Calculates the difference between 2 pointers
#define PtrDiff(x,y)                        (((PBYTE)(x))-((PBYTE)(y)))
#define PtrOffset(x,y)                      (((PBYTE)(x))+((QWORD)(y)))

// E.g: AlignAddressLower(0x1001,0x1000)   = 0x1000
//      AlignAddressUpper(0x1001,0x1000)   = 0x2000
#define AlignAddressLower(addr,alig)        ((QWORD)(addr)&~((QWORD)(alig)-1))
#define AlignAddressUpper(addr,alig)        AlignAddressLower(((QWORD)(addr)+(alig)-1), (alig))
#define IsAddressAligned(addr,alig)         (0==(((QWORD)(addr))&(((QWORD)(alig))-1)))

#define AddressOffset(addr,alig)            ((QWORD)(addr)&((alig)-1))

// check if a boundary is crossed
// i.e. addr and addr + size are in different memory regions (alignments)
#define IsInSameBoundary(addr,size,alig)    (AlignAddressLower((addr),(alig)) == \
                                             AlignAddressLower((QWORD)(addr)+(size)-1,(alig)))

// Natural alignment <- 2 * sizeof(PVOID)
// Because we only have x64 configuration => always 0x10
#define NATURAL_ALIGNMENT                   0x10

// These asserts can be very useful when working with hardware defined
// registers/structures to validate if the size of the defined C structure
// matches the hardware specification.
#define STATIC_ASSERT(Cond)                 STATIC_ASSERT_INFO(Cond,"")
#define STATIC_ASSERT_INFO(Cond,Msg)        static_assert(Cond,Msg)

// The stack must be aligned at 0x10 bytes - this must be done before
// the return address is pushed on the stack => the RA is aligned at 0x8 bytes
//
// push arg3
// push arg2
// push arg1
// push arg0
// call func  <-  rsp is 0x10 aligned

// func:
// mov  edi, edi  <- rsp is 0x8 aligned
#define IS_STACK_ALIGNED                    IsAddressAligned((PBYTE)_AddressOfReturnAddress()+sizeof(PVOID),NATURAL_ALIGNMENT)
#define CHECK_STACK_ALIGNMENT               ASSERT_INFO(IS_STACK_ALIGNED, "RSP at 0x%X\n", _AddressOfReturnAddress())
#define GET_RETURN_ADDRESS                  *((PVOID*)_AddressOfReturnAddress())

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (BYTE*)(address) - \
                                                  (QWORD)(&((type *)0)->field)))
#endif // CONTAINING_RECORD

#ifndef ARRAYSIZE
#define ARRAYSIZE(A)                            (sizeof(A)/sizeof((A)[0]))
#endif // ARRAYSIZE

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)               ((QWORD)&(((type *)0)->field))
#endif // FIELD_OFFSET

#define SECTOR_SIZE                             512

// because endianess this gets 0x55, 0xAA on disk :)
#define MBR_BOOT_SIGNATURE                      0xAA55

typedef BOOLEAN         INTR_STATE;

#define INTR_OFF        0
#define INTR_ON         1

// time related defines
#define SEC_IN_US       (1000*MS_IN_US)
#define MS_IN_US        1000

#define SEC_IN_NS       (1000*MS_IN_NS)
#define MS_IN_NS        (1000*US_IN_NS)
#define US_IN_NS        1000

// Checks if p      in [b, b + s]
//    and if p + ps in [b, b + s]
// Because ps and s are positive the actual check done is only for
// p >= b and p + ps <= b + s
#define CHECK_BOUNDS(p,ps,b,s)      ((((QWORD)(b) <= (QWORD)(p)) )&& (((QWORD)(p) + (QWORD)(ps) ) <= ((QWORD)(b)+(QWORD)(s))))
C_HEADER_END
