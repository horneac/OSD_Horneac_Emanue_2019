#pragma once

// VMX Basic Exit Reasons (Appendix C)

// Either:
//      1. Guest SW caused exception and the control bit was set to 1
//      2. NMI was delivered to processor and "NMI Exiting" is set
#define VM_EXIT_EXCEPTION_OR_NMI                        0

// An external interrupt was delivered and "external-interrupt exiting" is set
#define VM_EXIT_EXTERNAL_INT                            1

// Triple fault :)
#define VM_EXIT_TRIPLE_FAULT                            2

// An INIT signal arrived
#define VM_EXIT_INIT_SIGNAL                             3

// A SIPI signal arrived while processor was in "wait-for-SIPI" state
#define VM_EXIT_SIPI_SIGNAL                             4

// An SMI arrived after retirement of an I/O instruction (34.15.2)
#define VM_EXIT_IO_SMI_SIGNAL                           5

// An SMI arrived but not immediately after retirement of I/O instruction (34.15.2)
#define VM_EXIT_OTHER_SMI_SIGNAL                        6

// Interrupt Window
//      At the beginning of an instruction, RFLAGS.IF was 1, events were not blocked
//      by STI or MOV SS; and "interrupt-window exiting" was set
#define VM_EXIT_INT_WINDOW                              7

// NMI window
//      same as above; and "NMI-window exiting" was set
#define VM_EXIT_NMI_WINDOW                              8

// Guest SW attempted a task switch
#define VM_EXIT_TASK_SWITCH                             9

// Guest SW attempted to execute CPUID
#define VM_EXIT_CPUID                                   10

// Guest SW attempted to execute GETSEC
#define VM_EXIT_GETSEC                                  11

// Guest SW attempted to execute HLT and "HLT exiting" was set
#define VM_EXIT_HLT                                     12

// Guest SW attempted to execute INVD
#define VM_EXIT_INVD                                    13

// Guest SW attempted to execute INVLPG and "INVLPG exiting" was set
#define VM_EXIT_INVLPG                                  14

// Guest SW attempted to execute RDPMC and "RDPMC exiting" was set
#define VM_EXIT_RDPMC                                   15

// Guest SW attempted to execute RDTSC and "RDTSC exiting" was set
#define VM_EXIT_RDTSC                                   16

// Guest SW attempted to execute INVD in SMM
#define VM_EXIT_RSM                                     17

// VMCALL was executed either by guest SW or by the executive monitor (34.15.2)
#define VM_EXIT_VMCALL                                  18

// Guest SW attempted to execute VMCLEAR
#define VM_EXIT_VMCLEAR                                 19

// Guest SW attempted to execute VMLAUNCH
#define VM_EXIT_VMLAUNCH                                20

// Guest SW attempted to execute VMPTRLD
#define VM_EXIT_VMPTRLD                                 21

// Guest SW attempted to execute VMPTRST
#define VM_EXIT_VMPTRST                                 22

// Guest SW attempted to execute VMREAD
#define VM_EXIT_VMREAD                                  23

// Guest SW attempted to execute VMCLEAR
#define VM_EXIT_VMRESUME                                24

// Guest SW attempted to execute VMWRITE
#define VM_EXIT_VMWRITE                                 25

// Guest SW attempted to execute VMXOFF
#define VM_EXIT_VMXOFF                                  26

// Guest SW attempted to execute VMXON
#define VM_EXIT_VMXON                                   27

// Guest SW attempted to access CR0, CR3, CR4 or CR8 (25.1)
#define VM_EXIT_CR_ACCESS                               28

// Guest SW attempted a MOV to or from a debug register and the
// "MOV-DR exiting" was set
#define VM_EXIT_MOV_DR                                  29

// Guest SW attempted to execute an I/O Instruction and either:
//      1. "unconditional I/O exiting" was set
//      2. "use I/O bitmaps" was set and the bit associated with the port was set
#define VM_EXIT_IO_INSTRUCTION                          30

// Either:
//      1. "use MSR bitmaps" was 0
//      2. value of RCX not in a valid range( [0x0,0x1FFF] \/ [0xC0000000,0xC0001FFF] )
//      3. RCX was in low range and the n-th bit in read bitmap for low MSR was set
//      4. RCX was in high range and the n-th bit in read bitmap for high MSR was set
#define VM_EXIT_RDMSR                                   31

// Either:
//      1. "use MSR bitmaps" was 0
//      2. value of RCX not in a valid range( [0x0,0x1FFF] \/ [0xC0000000,0xC0001FFF] )
//      3. RCX was in low range and the n-th bit in write bitmap for low MSR was set
//      4. RCX was in high range and the n-th bit in write bitmap for high MSR was set
#define VM_EXIT_WRMSR                                   32

// A VM entry failed one of the checks (26.3.1)
#define VM_EXIT_ENTRY_FAILURE_GUEST_STATE               33

// A VM entry failed to load MSRs (26.4)
#define VM_EXIT_ENTRY_FAILURE_MSR_LOAD                  34

// Guest SW attempted to execute MWAIT and "MWAIT exiting" was set
#define VM_EXIT_MWAIT                                   36

// A VM entry occurred due to the 1-setting of the "monitor trap flag" and
// injection of an MTF VM exit as part of VM entry (25.5.2)
#define VM_EXIT_MONITOR_TRAP_FLAG                       37

// Guest SW attempted to execute MONITOR and "MONITOR exiting" was set
#define VM_EXIT_MONITOR                                 39

// Either:
//      1. Guest SW attempted to execute PAUSE and "PAUSE exiting" was set
//      2. "PAUSE-loop exiting" was set and guest SW executed a PAUSE loop with
//         execution time exceeding PLE_Window (25.1.3)
#define VM_EXIT_PAUSE                                   40

// A machine-check occurred during VM entry (26.8)
#define VM_EXIT_ENTRY_FAILURE_MACHINE_CHECK             41

// The TPR was below that of the TPR threshold field while "use TPR shadow"
// was set, either because:
//      1. As part of TPR virtualization (29.1.2)
//      2. As part of VM Entry (26.6.7)
#define VM_EXIT_TPR_BELOW_THRESHOLD                     43

// Guest SW attempted to access memory at a physical address on the
// APIC-access page and the "virtualize APIC accesses" was set (29.4)
#define VM_EXIT_APIC_ACCESS                             44

// EOI virtualization was performed for a virtual interrupt whose vector indexed
// a bit set in the EOI-exit bitmap
#define VM_EXIT_VIRTUALIZED_EOI                         45

// Guest SW attempted LGDT, LIDT, SGDT or SIDT and "descriptor-table exiting" was set
#define VM_EXIT_GDTR_IDTR_ACCESS                        46

// Guest SW attempted LLDT, LTR, SLDT or STR and "descriptor-table exiting" was set
#define VM_EXIT_LDTR_TR_ACCESS                          47

// An attempt to access memory with a guest-physical address was disallowed by the
// configuration of the EPT paging structures
#define EPT_VIOLATION_EXIT_QUALITIFICATION_READ_MASK            (1<<0)
#define EPT_VIOLATION_EXIT_QUALITIFICATION_WRITE_MASK           (1<<1)
#define EPT_VIOLATION_EXIT_QUALITIFICATION_EXEC_MASK            (1<<2)

#define EPT_VIOLATION_EXIT_QUALIFICATION_READ_PERMISSION        (1<<3)
#define EPT_VIOLATION_EXIT_QUALIFICATION_WRITE_PERMISSION       (1<<4)
#define EPT_VIOLATION_EXIT_QUALIFICATION_EXEC_PERMISSION        (1<<5)

#define EPT_VIOLATION_GUEST_LINEAR_ADDRESS_VALID_MASK           (1<<7)
#define EPT_VIOLATION_LINEAR_TO_PHYSICAL_TRANSLATION_MASK       (1<<8)

#define VM_EXIT_EPT_VIOLATION                           48

// An attempt to access memory with a guest-physical address encountered a misconfigured
// EPT paging-structure entry
#define VM_EXIT_EPT_MISCONFIGURATION                    49

// Guest SW attempted to execute INVEPT
#define VM_EXIT_INVEPT                                  50

// Guest SW attempted to execute RDTSCP and "enable RDTSCP" and "RDTSC exiting" were BOTH set
#define VM_EXIT_RDTSCP                                  51

// The preemption timer counted down to zero.
#define VM_EXIT_VMX_PREEMPT_TIMER_EXPIRED               52

// Guest SW attempted to execute INVVPID
#define VM_EXIT_INVVPID                                 53

// Guest SW attempted to execute WBINVD and "WBINVD exiting" was set
#define VM_EXIT_WBINVD                                  54

// Guest SW attempted to execute XSETBV
#define VM_EXIT_XSETBV                                  55

// Guest SW attempted to complete a write to the virtual-APIC page that
// must be virtualized by VMM SW (29.4.3.3)
#define VM_EXIT_APIC_WRITE                              56

// Guest SW attempted to execute RDRAND and "RDRAND exiting" was set
#define VM_EXIT_RDRAND                                  57

// Guest SW attempted to execute INVPCID and "enable INVPCID" and "INVLPG exiting" were BOTH set
#define VM_EXIT_INVPCID                                 58

// Guest SW invoked a VM function and either:
//      1. The function invoked was not enabled
//      2. The function generated a function-specific condition causing a VM exit
#define VM_EXIT_VMFUNC                                  59

// Guest SW attempted to execute RDSEED and "RDSEED exiting" was set
#define VM_EXIT_RDSEED                                  61

// Guest SW attempted to execute XSAVES and "enable XSAVES/XRSTORS" was set
// and a bit was set in the logical-AND of the following 3 values:
// EDX:EAX, IA32_XSS MSR, XSS-exiting bitmap
#define VM_EXIT_XSAVES                                  63

// Guest SW attempted to execute XRSTORS and "enable XSAVES/XRSTORS" was set
// and a bit was set in the logical-AND of the following 3 values:
// EDX:EAX, IA32_XSS MSR, XSS-exiting bitmap
#define VM_EXIT_XRSTORS                                 64

#define VM_EXIT_RESERVED                                VM_EXIT_XRSTORS + 1
