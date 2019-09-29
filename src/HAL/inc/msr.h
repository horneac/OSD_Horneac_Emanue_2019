#pragma once

#define     MSR_DATA_SIZE                       0x8

// specifies the filter size of the MONITOR instruction
#define     IA32_MONITOR_FILTER_SIZE_MSR            0x00000006

#define     IA32_APIC_BASE_MSR                  0x0000001B

// IA32_FEATURE_CONTROL

// Bit 20       - LMCE On
// Bit 15       - SENTER global enable
// Bits 14:8    - SENTER local function enable
// Bit 2        - Enable VMX outside SMX operation
// Bit 1        - Enable VMX inside SMX operation
// Bit 0        - If 1 => locked => cannot write to this MSR
#define     IA32_FEATURE_VMX_OUTSIDE_SMX        ((QWORD)1<<2)
#define     IA32_FEATURE_VMX_INSIDE_SMX         ((QWORD)1<<1)
#define     IA32_FEATURE_LOCKED                 ((QWORD)1<<0)

#define     IA32_FEATURE_CONTROL                0x0000003A

#define     IA32_PMC0                           0x000000C1
// ...
#define     IA32_PMC7                           0x000000C8

// IA32_MTRRCAP

#define     IA32_MTRRCAP                        0x000000FE

#define     IA32_SYSENTER_CS                    0x00000174
#define     IA32_SYSENTER_ESP                   0x00000175
#define     IA32_SYSENTER_EIP                   0x00000176

#define     IA32_PERFEVTSEL0                    0x00000186

// IA32_MISC_ENABLE
#define     IA32_MISC_ENABLE_PERF_MON_AVL       ((QWORD)1<<7)
#define     IA32_MISC_ENABLE_BTS_UNAVL          ((QWORD)1<<11)
#define     IA32_MISC_ENABLE_PEBS_UNAVL         ((QWORD)1<<12)

#define     IA32_MISC_ENABLE                    0x000001A0

// MSR_LBR_SELECT
#define     IA32_MSR_LBR_SELECT                 0x000001C8
#define     IA32_MSR_LASTBRANCH_TOS             0x000001C9

#define     IA32_DEBUGCTL_LBR                       ((QWORD)1<<0)
#define     IA32_DEBUGCTL_TR                        ((QWORD)1<<6)
#define     IA32_DEBUGCTL_BTS                       ((QWORD)1<<7)
#define     IA32_DEBUGCTL_BTINT                     ((QWORD)1<<8)
#define     IA32_DEBUGCTL_BTS_OFF_OS                ((QWORD)1<<9)
#define     IA32_DEBUGCTL_BTS_OFF_USR               ((QWORD)1<<10)
#define     IA32_DEBUGCTL_FREEZE_LBR_ON_PMI         ((QWORD)1<<11)
#define     IA32_DEBUGCTL_FREEZE_PERF_MON_ON_PMI    ((QWORD)1<<12)

#define     IA32_DEBUGCTL                       0x000001D9

// IA32_MTRR_PHYSBASE0
// IA32_MTRR_PHYSMASK0
// each variable length MTRR register is mapped in pair
// and currently(Vol 30, Sept 14) there are until register9

#define     IA32_MTRR_BASE_MEMORY_TYPE          (MAX_BYTE)

#define     IA32_MTRR_PHYSBASE0                 0x00000200

#define     IA32_MTRR_MASK_VALID_MASK           ((QWORD)1<<11)

#define     IA32_MTRR_PHYSMASK0                 0x00000201


#define     IA32_MTRR_PHYSBASE9                 0x00000212
#define     IA32_MTRR_PHYSMASK9                 0x00000213

// IA32_MTRR_FIX64K_00000

#define     IA32_MTRR_FIX64K_00000              0x00000250

// IA32_MTRR_FIX16K_80000

#define     IA32_MTRR_FIX16K_80000              0x00000258

// IA32_MTRR_FIX16K_A0000

#define     IA32_MTRR_FIX16K_A0000              0x00000259

// IA32_MTRR_FIX4K_C0000

#define     IA32_MTRR_FIX4K_C0000               0x00000268

// IA32_MTRR_FIX4K_C8000

#define     IA32_MTRR_FIX4K_C8000               0x00000269

// IA32_MTRR_FIX4K_D0000

#define     IA32_MTRR_FIX4K_D0000               0x0000026A

// IA32_MTRR_FIX4K_D8000

#define     IA32_MTRR_FIX4K_D8000               0x0000026B

// IA32_MTRR_FIX4K_E0000

#define     IA32_MTRR_FIX4K_E0000               0x0000026C

// IA32_MTRR_FIX4K_E8000

#define     IA32_MTRR_FIX4K_E8000               0x0000026D

// IA32_MTRR_FIX4K_F0000

#define     IA32_MTRR_FIX4K_F0000               0x0000026E

// IA32_MTRR_FIX4K_F8000

#define     IA32_MTRR_FIX4K_F8000               0x0000026F

// IA32_PAT

#define     IA32_PAT                            0x00000277


// IA32_MTRR_DEF_TYPE
#define     IA32_MTRR_DEFAULT_MEMORY_MASK       ((QWORD)0x7)
#define     IA32_FIXED_MTRR_ENABLE_MASK         ((QWORD)1<<10)
#define     IA32_MTRR_ENABLE_MASK               ((QWORD)1<<11)

#define     IA32_MTRR_DEF_TYPE                  0x000002FF

// PERF GLOBAL CONTROL
#define     IA32_PERF_GLOBAL_CTRL_ENABLE_PMC_BIT_BASE   0

#define     IA32_PERF_CAPABILITIES                  0x00000345
#define     IA32_FIXED_CTR_CTRL                     0x0000038D
#define     IA32_PERF_GLOBAL_STATUS                 0x0000038E
#define     IA32_PERF_GLOBAL_CTRL                   0x0000038F
#define     IA32_PERF_GLOBAL_OVF_CTRL               0x00000390
#define     IA32_PERF_GLOBAL_STATUS_SET             0x00000391
#define     IA32_PERF_GLOBAL_INUSE                  0x00000392
#define     IA32_PEBS_ENABLE                        0x000003F1

//
//
// VMX based MSR's
//
//

// BASIC VMX Information (A.1)

// Bit 55       - If 1 => supports TRUE based MSRs
// Bit 54       - If 1 => VMEXIT due to INS/OUTS reported in
//                VM-exit instruction-information field
// Bits 53:50   - Memory type used to access VMCS and structures
//                referenced by it
//                  Value       Field
//                  0           UC
//                  6           WB
// Bit 49       - If 1 => cpu supports dual-monitor etc...
// Bit 48       - If 0 => addresses limited to processor physical width
// Bits 44:32   - number of bytes to allocate for VMXON/VMCS region
// Bit 31       - always 0
// Bits 30:0    - 31 bit VMCS revision identifier
#define     IA32_VMX_BASIC_MSR                  0x00000480

// VM Execution Controls (A.3)

// Pin-Base VM-Execution Controls (A.3.1)

// if MSR[IA32_VMX_BASIC_MSR].55 = 1 => this MSR is useless
//                                      read IA32_VMX_TRUE_PINBASED_CTLS MSR
// Bits 63:32   - allows control X to be 1 if bit (X+32) in the MSR is cleared to 0
// Bits 31:0    - allows control X to be 0 if bit X in the MSR is cleared to 0
#define     IA32_VMX_PINBASED_CTLS              0x00000481

// Primary Processor-Based VM-Execution Controls (A.3.2)

// Same as for IA32_VMX_PINBASED_CTLS
// if MSR[IA32_VMX_BASIC_MSR].55 = 1 => read IA32_VMX_TRUE_PROCBASED_CTLS MSR
#define     IA32_VMX_PROCBASED_CTLS             0x00000482

// VM-EXIT Controls (A.4)

// Same as for IA32_VMX_PINBASED_CTLS
// if MSR[IA32_VMX_BASIC_MSR].55 = 1 => read IA32_VMX_TRUE_EXIT_CTLS MSR
#define     IA32_VMX_EXIT_CTLS                  0x00000483

// VM-ENTRY Control (A.5)

// Same as for IA32_VMX_PINBASED_CTLS
// if MSR[IA32_VMX_BASIC_MSR].55 = 1 => read IA32_VMX_TRUE_ENTRY_CTLS MSR
#define     IA32_VMX_ENTRY_CTLS                 0x00000484

// MISCELLANEOUS Data (A.6)

// Bits 63:32   - 32-bit MSEG revision identifier
// Bit 29       - If 1 => SW can use VMWRITE to any supported field in VMCS
// Bit 28       - If 1 => TODO (SMM related)
// Bits 27:25   - Maximum number of MSR in store-list or load-list
//                  Max_MSRs = 512 * ( N + 1 )
// Bits 24:16   - number of CR3-target values supported by the processor
// Bit 15       - If 1 => RDMSR can be used in SMM...
// Bits 8:6     - A bitmap of activity states supported
//                  Bit 6 => support for activity state 1 (HLT)
//                  Bit 7 => support for activity state 2 (shutdown)
//                  Bit 8 => support for activity state 3 (wait-for-SIPI)
// Bit 5        - If 1 => IA32_EFER.LMA stored into "IA-32e mode guest"
//                VM-Entry control
// Bits 4:0     - VMX-preemption timer counts down by 1 every time bit X in
//                the TSC changes due to a TSC increment
#define     IA32_VMX_MISC                       0x00000485

// VMX-Fixed Bits in CR0 (A.7)

// Flexible if 0 in MSR[IA32_VMX_CRO_FIXED0].X and
//             1 in MSR[IA32_VMX_CRO_FIXED1].X

// If 1 => bit fixed to 1 in VMX operation
#define     IA32_VMX_CRO_FIXED0                 0x00000486

// If 0 => bit fixed to 0 in VMX operation
#define     IA32_VMX_CRO_FIXED1                 0x00000487

// VMX-Fixed bits in CR4 (A.8)

// Flexible if 0 in MSR[IA32_VMX_CR4_FIXED0].X and
//             1 in MSR[IA32_VMX_CR4_FIXED1].X

// If 1 => bit fixed to 1 in VMX operation
#define     IA32_VMX_CR4_FIXED0                 0x00000488

// If 0 => bit fixed to 0 in VMX operation
#define     IA32_VMX_CR4_FIXED1                 0x00000489

// VMCS Enumeration (A.9)

// Bits 14:13   - indicate the field's width
// Bits 11:10   - indicate the field's type
// Bits 9:1     - contains the highest index value used for
//                any VMCS encoding
#define     IA32_VMX_VMCS_ENUM                  0x0000048A

// Secondary Processor-Based VM-Execution Controls (A.3.3)

// Exists only if MSR[IA32_VMX_PROCBASED_CTLS].63 = 1
// Same functioning as IA32_VMX_PROCBASED_CTLS
#define     IA32_VMX_PROCBASED_CTLS2            0x0000048B

// VPID and EPT capabilities (A.10)

// Bit 43       - If 1 => single-context-retaining-globals INVVPID support
// Bit 42       - If 1 => all-context INVVPID support
// Bit 41       - If 1 => single-context INVVPID support
// Bit 40       - If 1 => individual-address INVVPID support
// Bit 32       - If 1 => INVVPID support
// Bit 26       - If 1 => all-context INVEPT supported
// Bit 25       - If 1 => single-context INVEPT supported
// Bit 21       - If 1 => A&D flags for EPT supported
// Bit 20       - If 1 => INVEPT support
// Bit 17       - If 1 => EPT PDPTE can map 1-Gbyte page
// Bit 16       - If 1 => EPT PDE can map 2-Mbyte page
// Bit 14       - If 1 => EPT paging-structure can use WB MT
// Bit 8        - If 1 => EPT paging-structure can use UC memory
// Bit 6        - Support for page-walk length of 4
// Bit 0        - If 1 => allows execute-only EPT mapping
#define     IA32_VMX_EPT_VPID_CAP               0x0000048C

#define     IA32_VMX_TRUE_PINBASED_CTLS         0x0000048D
#define     IA32_VMX_TRUE_PROCBASED_CTLS        0x0000048E
#define     IA32_VMX_TRUE_EXIT_CTLS             0x0000048F
#define     IA32_VMX_TRUE_ENTRY_CTLS            0x00000490

#define     IA32_DS_AREA                        0x00000600

// VM Functions (A.11)

// Exists only if       MSR[IA32_VMX_PROCBASED_CTLS ].63 = 1
//                  AND MSR[IA32_VMX_PROCBASED_CTLS2].45 = 1

// If bit X is set in MSR => VMFUNC for X is allowed
#define     IA32_VMX_VMFUNC                     0x00000491

#define     IA32_EFER_SCE                       ((QWORD)1<<0)
#define     IA32_EFER_LME                       ((QWORD)1<<8)
#define     IA32_EFER_LMA                       ((QWORD)1<<10)
#define     IA32_EFER_NXE                       ((QWORD)1<<11)

#define     IA32_EFER                           0xC0000080

#pragma pack(push,1)

#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

typedef union _IA32_STAR_MSR_DATA
{
    struct
    {
        DWORD           __Reserved0;
        WORD            SyscallCsDs;
        WORD            SysretCsDs;
    };
    QWORD               Raw;
} IA32_STAR_MSR_DATA, *PIA32_STAR_MSR_DATA;
STATIC_ASSERT(sizeof(IA32_STAR_MSR_DATA) == MSR_DATA_SIZE);

#pragma warning(pop)
#pragma pack(pop)

#define     IA32_STAR                           0xC0000081
#define     IA32_LSTAR                          0xC0000082
#define     IA32_FMASK                          0xC0000084

// FS_BASE MSR
#define     IA32_FS_BASE_MSR                    0xC0000100

// GS_BASE MSR
#define     IA32_GS_BASE_MSR                    0xC0000101

// IA32_KERNEL_GS_BASE
#define     IA32_KERNEL_GS_BASE                 0xC0000102

#define     IA32_TSC_AUX                        0xC0000103