#pragma once

#include "vmx_exit_reasons.h"

#pragma pack(push,1)

#pragma warning(push)

//warning C4214: nonstandard extension used : bit field types other than int
#pragma warning(disable:4214)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

#define CR_ACCESS_TYPE_MOV_TO_CR                           0
#define CR_ACCESS_TYPE_MOVE_FROM_CR                        1
#define CR_ACCESS_TYPE_CLTS                                2
#define CR_ACCESS_TYPE_LMSW                                3

typedef struct _EXIT_QUALIFICATION_CR_ACCESS
{
    QWORD           ControlRegisterNumber               :   4;
    QWORD           AccessType                          :   2;
    QWORD           LMSWOperandType                     :   1;
    QWORD           Reserved0                           :   1;
    QWORD           GeneralPurposeRegister              :   4;
    QWORD           Reserved1                           :   4;
    QWORD           LMSWSourceData                      :  16;
    QWORD           Reserved2                           :  32;
} EXIT_QUALIFICATION_CR_ACCESS, *PEXIT_QUALIFICATION_CR_ACCESS;
STATIC_ASSERT( sizeof( EXIT_QUALIFICATION_CR_ACCESS ) == sizeof( QWORD ) );

#define IO_ACCESS_SIZE_1BYTE                                0
#define IO_ACCESS_SIZE_2BYTES                               1
#define IO_ACCESS_SIZE_4BYTES                               3

#define IO_ACCESS_DIRECTION_OUT                             0
#define IO_ACCESS_DIRECTION_IN                              1

#define IO_ACCESS_OPERAND_DX                                0
#define IO_ACCESS_OPERAND_IMMEDIATE                         1

typedef struct _EXIT_QUALIFICATION_IO_INSTRUCTION
{
    QWORD           AccessSize                          :   3;
    QWORD           AccessDirection                     :   1;
    QWORD           StringOperation                     :   1;
    QWORD           RepPrefixed                         :   1;
    QWORD           OperandEncoding                     :   1;
    QWORD           Reserved0                           :   9;
    QWORD           PortNumber                          :  16;
    QWORD           Reserved1                           :  32;
} EXIT_QUALIFICATION_IO_INSTRUCTION, *PEXIT_QUALIFICATION_IO_INSTRUCTION;
STATIC_ASSERT( sizeof( EXIT_QUALIFICATION_IO_INSTRUCTION ) == sizeof( QWORD ) );

// 4 - VM_SIPI_SIGNAL
typedef struct _EXIT_QUALIFICATION_SIPI
{
    QWORD           Vector                              :   8;
    QWORD           Reserved                            :  56;
} EXIT_QUALIFICATION_SIPI, *PEXIT_QUALIFICATION_SIPI;
STATIC_ASSERT( sizeof( EXIT_QUALIFICATION_SIPI ) == sizeof( QWORD ) );

// Table 27-10. Format of the VM-Exit Instruction-Information Field as Used for LIDT, LGDT, SIDT, or SGDT
// 46 - LIDT/LGDT/SIDT/SGDT
#define ADDRESS_SIZE_16_BIT                                 0b000
#define ADDRESS_SIZE_32_BIT                                 0b001
#define ADDRESS_SIZE_64_BIT                                 0b010

#define INSTRUCTION_IDENTITY_SGDT                           0b00
#define INSTRUCTION_IDENTITY_SIDT                           0b01
#define INSTRUCTION_IDENTITY_LGDT                           0b10
#define INSTRUCTION_IDENTITY_LIDT                           0b11

typedef union _EXIT_INSTRUCTION_INFORMATION_GDT_IDT
{
    struct
    {
        // Undefined for instructions with no index register (bit 22 is set).
        DWORD           Scaling                             :   2;
        DWORD           __Reserved0                         :   5;
        DWORD           AddressSize                         :   3;
        DWORD           Zero                                :   1;

        // Undefined for VM exits from 64-bit mode
        DWORD           OperandSize                         :   1;
        DWORD           __Reserved1                         :   3;
        DWORD           SegmentRegister                     :   3;

        // Undefined for instructions with no index register (bit 22 is set)
        DWORD           IndexRegister                       :   4;
        DWORD           IndexRegInvalid                     :   1;

        // Undefined for instructions with no base register (bit 27 is set)
        DWORD           BaseRegister                        :   4;
        DWORD           BaseRegisterInvalid                 :   1;
        DWORD           InstructionIdentity                 :   2;
        DWORD           __Reserved2                         :   2;
    };
    DWORD               Raw;
} EXIT_INSTRUCTION_INFORMATION_GDT_IDT, *PEXIT_INSTRUCTION_INFORMATION_GDT_IDT;
STATIC_ASSERT( sizeof( EXIT_INSTRUCTION_INFORMATION_GDT_IDT ) == sizeof( DWORD ) );

// 47 - LLDT/LTR/SLDT/STR
typedef union _EXIT_INSTRUCTION_INFORMATION_LDT_TR
{
    struct
    {
        // Undefined for register instructions (bit 10 is set) and for memory instructions with no index register
        // (bit 10 is clear and bit 22 is set).
        DWORD           Scaling                             :   2;
        DWORD           __Reserved0                         :   1;

        // Undefined for memory instructions (bit 10 is clear)
        DWORD           Register1                           :   4;

        // Other values not used. Undefined for register instructions (bit 10 is set).
        DWORD           AddressSize                         :   3;

        // Mem/Reg (0 = memory; 1 = register).
        DWORD           MemReg                              :   1;
        DWORD           __Reserved11_14                     :   4;

        // Undefined for register instructions (bit 10 is set)
        DWORD           SegmentRegister                     :   3;

        // Undefined for register instructions(bit 10 is set) and for memory instructions with no index register
        // (bit 10 is clear and bit 22 is set).
        DWORD           IndexReg                            :   4;
        DWORD           IndexRegInvalid                     :   1;

        // Undefined for register instructions(bit 10 is set) and for memory instructions with no base register
        // (bit 10 is clear and bit 27 is set)
        DWORD           BaseRegister                        :   4;
        DWORD           BaseRegisterInvalid                 :   1;

        DWORD           InstructionIdentity                 :   2;
        DWORD           __Reserved2                         :   2;
    };
    DWORD               Raw;
} EXIT_INSTRUCTION_INFORMATION_LDT_TR, *PEXIT_INSTRUCTION_INFORMATION_LDT_TR;
STATIC_ASSERT( sizeof( EXIT_INSTRUCTION_INFORMATION_LDT_TR ) == sizeof( DWORD ) );

typedef enum _OPERAND_SIZE
{
    OperandSizeWord,
    OperandSizeDword,
    OperandSizeQword,
    OperandSizeUnused
} OPERAND_SIZE, *POPERAND_SIZE;

// 57 - VM_EXIT_RDRAND
typedef union _EXIT_INSTRUCTION_INFORMATION_RD_INSTRUCTION
{
    struct
    {
        DWORD           Undefined0                          :   3;
        DWORD           DestinationRegister                 :   4;
        DWORD           Undefined1                          :   4;
        DWORD           OperandSize                         :   2;
        DWORD           Undefined2                          :  19;
    };
    DWORD               Raw;
} EXIT_INSTRUCTION_INFORMATION_RD_INSTRUCTION, *PEXIT_INSTRUCTION_INFORMATION_RD_INSTRUCTION;
STATIC_ASSERT( sizeof( EXIT_INSTRUCTION_INFORMATION_RD_INSTRUCTION ) == sizeof( DWORD ) );

// Event-specific information is provided for VM exits due to the following vectored events: exceptions (including
// those generated by the instructions INT3, INTO, BOUND, and UD2); external interrupts that occur while the
// “acknowledge interrupt on exit” VM - exit control is 1; and non - maskable interrupts(NMIs).
typedef union _EXIT_INTERRUPTION_INFORMATION
{
    struct
    {
        DWORD           Vector                              :   8;
        DWORD           Type                                :   3;
        DWORD           ErrorCodeValid                      :   1;
        DWORD           NMIUnblockingDueToIRET              :   1;
        DWORD           __Reserved                          :  18;
        DWORD           Valid                               :   1;
    };
    DWORD               Raw;
} EXIT_INTERRUPTION_INFORMATION, *PEXIT_INTERRUPTION_INFORMATION;
STATIC_ASSERT( sizeof( EXIT_INTERRUPTION_INFORMATION ) == sizeof( DWORD ) );

#pragma warning(pop)
#pragma pack(pop)
