#pragma once

// Control and system registers related information

// CR0 related definitions
#define CR0_PE                                      ((QWORD)1<<0)
#define CR0_EM                                      ((QWORD)1<<2)
#define CR0_ET                                      ((QWORD)1<<4)
#define CR0_NE                                      ((QWORD)1<<5)
#define CR0_WP                                      ((QWORD)1<<16)
#define CR0_NW                                      ((QWORD)1<<29)
#define CR0_CD                                      ((QWORD)1<<30)
#define CR0_PG                                      ((QWORD)1<<31)

// CR4 related definitions
#define CR4_PAE                                     ((QWORD)1<<5)
#define CR4_OSFXSR                                  ((QWORD)1<<9)
#define CR4_OSXMMEXCPT                              ((QWORD)1<<10)
#define CR4_VMXE                                    ((QWORD)1<<13)
#define CR4_SMXE                                    ((QWORD)1<<14)
#define CR4_PCIDE                                   ((QWORD)1<<17)
#define CR4_OSXSAVE                                 ((QWORD)1<<18)
#define CR4_SMEP                                    ((QWORD)1<<20)
#define CR4_SMAP                                    ((QWORD)1<<21)

// Extended control registers
#define XCR0_INDEX                                  0

// RFLAGS related definitions
#define RFLAGS_CARRY_FLAG_BIT                       ((QWORD)1<<0)
#define RFLAGS_RESERVED_BIT                         ((QWORD)1<<1)
#define RFLAGS_TRAP_BIT                             ((QWORD)1<<8)
#define RFLAGS_INTERRUPT_FLAG_BIT                   ((QWORD)1<<9)
#define RFLAGS_DIRECTION_BIT                        ((QWORD)1<<10)
#define RFLAGS_RESUME_FLAG_BIT                      ((QWORD)1<<16)

typedef enum
{
    RegisterRax,
    RegisterRcx,
    RegisterRdx,
    RegisterRbx,
    RegisterRsp,
    RegisterRbp,
    RegisterRsi,
    RegisterRdi,
    RegisterR8,
    RegisterR9,
    RegisterR10,
    RegisterR11,
    RegisterR12,
    RegisterR13,
    RegisterR14,
    RegisterR15
} GeneralPurposeRegisterIndexes;

typedef enum
{
    CR0,
    CR1,
    CR2,
    CR3,
    CR4,
    CR5,
    CR6,
    CR7,
    CR8
} ControlRegisterIndexes;


typedef enum
{
    SelectorFirst,

    SelectorES = SelectorFirst,
    SelectorCS,
    SelectorSS,
    SelectorDS,
    selectorFS,
    SelectorGS,
    SelectorTR,

    SelectorReserved = SelectorTR + 1
} SelectorIndex;
