#pragma once

typedef DWORD VMCS_FIELD;

#pragma pack(push,1)

#pragma warning(push)

// warning C4214: nonstandard extension used : bit field types other than int
#pragma warning(disable:4214)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

// VMCS field encoding:
// Bits 31:15       - Reserved
// Bits 14:13       - Width
//                      0: 16-bit
//                      1: 64-bit
//                      2: 32-bit
//                      3: natural-width
// Bit 12           - Reserved
// Bits 11:10       - Type
//                      0: control
//                      1: VM-exit information
//                      2: guest state
//                      3: host state
// Bits 9:1         - Index
// Bit 0            - Access Type( 0 = full; 1 = high )

#define VMCS_FIELD_16_BIT                               0x00000000UL
#define VMCS_FIELD_64_BIT                               0x00002000UL
#define VMCS_FIELD_32_BIT                               0x00004000UL
#define VMCS_FIELD_NATURAL                              0x00006000UL

#define VMCS_TYPE_CONTROL                               0x00000000UL
#define VMCS_TYPE_EXIT                                  0x00000400UL
#define VMCS_TYPE_GUEST                                 0x00000800UL
#define VMCS_TYPE_HOST                                  0x00000C00UL

#define VMCS_ACCESS_FULL                                0x00000000UL
#define VMCS_ACCESS_HIGH                                0x00000001UL


// 16-Bit Fields (B.1)

// 16-Bit Control Fields (B.1.1)

// Only with "enable VPID" support
#define VMCS_CONTROL_VPID_IDENTIFIER                        (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_CONTROL | ( 0 << 1 ) | VMCS_ACCESS_FULL )

// Only with "process posted interrupts" support
#define VMCS_CONTROL_POSTED_INTERRUPT_NOTIFY_VECTOR         (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_CONTROL | ( 1 << 1 ) | VMCS_ACCESS_FULL )

// Only with "EPT-violation #VE" support
#define VMCS_CONTROL_EPTP_INDEX                             (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_CONTROL | ( 2 << 1 ) | VMCS_ACCESS_FULL )

// 16-Bit Guest State Fields (B.1.2)

#define VMCS_GUEST_ES_SELECTOR                              (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_GUEST | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_CS_SELECTOR                              (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_GUEST | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_SS_SELECTOR                              (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_GUEST | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_DS_SELECTOR                              (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_GUEST | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_FS_SELECTOR                              (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_GUEST | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_GS_SELECTOR                              (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_GUEST | ( 5 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_LDTR_SELECTOR                            (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_GUEST | ( 6 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_TR_SELECTOR                              (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_GUEST | ( 7 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_INT_STATUS                               (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_GUEST | ( 8 << 1 ) | VMCS_ACCESS_FULL )

// 16-Bit Host State Fields (B.1.3)

#define VMCS_HOST_ES_SELECTOR                               (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_HOST | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_CS_SELECTOR                               (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_HOST | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_SS_SELECTOR                               (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_HOST | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_DS_SELECTOR                               (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_HOST | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_FS_SELECTOR                               (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_HOST | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_GS_SELECTOR                               (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_HOST | ( 5 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_TR_SELECTOR                               (VMCS_FIELD)( VMCS_FIELD_16_BIT | VMCS_TYPE_HOST | ( 6 << 1 ) | VMCS_ACCESS_FULL )

// 64-Bit Fields (B.2)

// 64-Bit Control Fields (B.2.1)

#define VMCS_CONTROL_IO_BITMAP_A_ADDRESS_FULL               (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_IO_BITMAP_A_ADDRESS_HIGH               (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 0 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_IO_BITMAP_B_ADDRESS_FULL               (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_IO_BITMAP_B_ADDRESS_HIGH               (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 1 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_MSR_BITMAP_ADDRESS_FULL                (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_MSR_BITMAP_ADDRESS_HIGH                (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 2 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_MSR_STORE_EXIT_ADDRESS_FULL            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_MSR_STORE_EXIT_ADDRESS_HIGH            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 3 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_MSR_LOAD_EXIT_ADDRESS_FULL             (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_MSR_LOAD_EXIT_ADDRESS_HIGH             (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 4 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_MSR_LOAD_ENTRY_ADDRESS_FULL            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 5 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_MSR_LOAD_ENTRY_ADDRESS_HIGH            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 5 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_EXECUTIVE_VMCS_POINTER_FULL            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 6 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_EXECUTIVE_VMCS_POINTER_HIGH            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 6 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_TSC_OFFSET_FULL                        (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 8 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_TSC_OFFSET_HIGH                        (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 8 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_VIRTUAL_APIC_ADDRESS_FULL              (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 9 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_VIRTUAL_APIC_ADDRESS_HIGH              (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 9 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_APIC_ACCESS_ADDRESS_FULL               (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 10 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_APIC_ACCESS_ADDRESS_HIGH               (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 10 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_POSTED_INT_DESC_ADDRESS_FULL           (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 11 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_POSTED_INT_DESC_ADDRESS_HIGH           (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 11 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_VM_FUNC_CONTROLS_FULL                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 12 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_VM_FUNC_CONTROLS_HIGH                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 12 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_EPT_POINTER_FULL                       (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 13 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_EPT_POINTER_HIGH                       (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 13 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_EOI_EXIT0_BITMAP_FULL                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 14 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_EOI_EXIT0_BITMAP_HIGH                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 14 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_EOI_EXIT1_BITMAP_FULL                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 15 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_EOI_EXIT1_BITMAP_HIGH                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 15 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_EOI_EXIT2_BITMAP_FULL                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 16 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_EOI_EXIT2_BITMAP_HIGH                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 16 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_EOI_EXIT3_BITMAP_FULL                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 17 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_EOI_EXIT3_BITMAP_HIGH                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 17 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_EPTP_LIST_ADDRESS_FULL                 (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 18 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_EPTP_LIST_ADDRESS_HIGH                 (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 18 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_VMREAD_BITMAP_ADDRESS_FULL             (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 19 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_VMREAD_BITMAP_ADDRESS_HIGH             (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 19 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_VMWRITE_BITMAP_ADDRESS_FULL            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 20 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_VMWRITE_BITMAP_ADDRESS_HIGH            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 20 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_VE_INFO_ADDRESS_FULL                   (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 21 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_VE_INFO_ADDRESS_HIGH                   (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 21 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_CONTROL_XSS_EXISTING_BITMAP_FULL               (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 22 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_XSS_EXISTING_BITMAP_HIGH               (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_CONTROL | ( 22 << 1 ) | VMCS_ACCESS_HIGH )


// 64-Bit Exit Fields (B.2.2)

// Only with "enable EPT" support
#define VMCS_EXIT_GUEST_PHYSICAL_ADDR_FULL                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_EXIT | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_GUEST_PHYSICAL_ADDR_HIGH                  (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_EXIT | ( 0 << 1 ) | VMCS_ACCESS_HIGH )


// 64-Bit Guest-State Fields (B.2.3)

#define VMCS_GUEST_VMCS_LINK_POINTER_FULL                   (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_VMCS_LINK_POINTER_HIGH                   (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 0 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_GUEST_IA32_DEBUGCTL_FULL                       (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_IA32_DEBUGCTL_HIGH                       (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 1 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_GUEST_IA32_PAT_FULL                            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_IA32_PAT_HIGH                            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 2 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_GUEST_IA32_EFER_FULL                           (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_IA32_EFER_HIGH                           (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 3 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_GUEST_IA32_PERF_GLOBAL_CTRL_FULL               (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_IA32_PERF_GLOBAL_CTRL_HIGH               (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 4 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_GUEST_PDPTE0_FULL                              (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 5 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_PDPTE0_HIGH                              (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 5 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_GUEST_PDPTE1_FULL                              (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 6 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_PDPTE1_HIGH                              (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 6 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_GUEST_PDPTE2_FULL                              (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 7 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_PDPTE2_HIGH                              (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 7 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_GUEST_PDPTE3_FULL                              (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 8 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_PDPTE3_HIGH                              (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_GUEST | ( 8 << 1 ) | VMCS_ACCESS_HIGH )

// 64-Bit Host-State Fields (B.2.4)

#define VMCS_HOST_IA32_PAT_FULL                             (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_HOST | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_IA32_PAT_HIGH                             (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_HOST | ( 0 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_HOST_IA32_EFER_FULL                            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_HOST | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_IA32_EFER_HIGH                            (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_HOST | ( 1 << 1 ) | VMCS_ACCESS_HIGH )

#define VMCS_HOST_IA32_PERF_GLOBAL_CTRL_FULL                (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_HOST | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_IA32_PERF_GLOBAL_CTRL_HIGH                (VMCS_FIELD)( VMCS_FIELD_64_BIT | VMCS_TYPE_HOST | ( 2 << 1 ) | VMCS_ACCESS_HIGH )


// 32-Bit Fields (B.3)

// 32-Bit Control Fields (B.3.1)

// Pin-Based VM-Execution Controls
#define PIN_BASED_EXTERNAL_INT_EXIT                         ((QWORD)1<<0)
#define PIN_BASED_NMI_EXIT                                  ((QWORD)1<<3)
#define PIN_BASED_VIRTUAL_NMI                               ((QWORD)1<<5)
#define PIN_BASED_ACTIVATE_PREEMPT_TIMER                    ((QWORD)1<<6)
#define PIN_BASED_PROCESS_POSTED_INTS                       ((QWORD)1<<7)

#define VMCS_CONTROL_PINBASED_CONTROLS                      (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 0 << 1 ) | VMCS_ACCESS_FULL )

// Primary Processor-Based VM-Execution Controls
#define PROC_BASED_PRIMARY_INT_WINDOW_EXIT                  ((QWORD)1<<2)
#define PROC_BASED_PRIMARY_TSC_OFFSETING                    ((QWORD)1<<3)
#define PROC_BASED_PRIMARY_HLT_EXIT                         ((QWORD)1<<7)
#define PROC_BASED_PRIMARY_INVLPG_EXIT                      ((QWORD)1<<9)
#define PROC_BASED_PRIMARY_MWAIT_EXIT                       ((QWORD)1<<10)
#define PROC_BASED_PRIMARY_RDPMC_EXIT                       ((QWORD)1<<11)
#define PROC_BASED_PRIMARY_RDTSC_EXIT                       ((QWORD)1<<12)
#define PROC_BASED_PRIMARY_CR3_LOAD_EXIT                    ((QWORD)1<<15)
#define PROC_BASED_PRIMARY_CR3_STORE_EXIT                   ((QWORD)1<<16)
#define PROC_BASED_PRIMARY_CR8_LOAD_EXIT                    ((QWORD)1<<19)
#define PROC_BASED_PRIMARY_CR8_STORE_EXIT                   ((QWORD)1<<20)
#define PROC_BASED_PRIMARY_USE_TPR_SHADOW                   ((QWORD)1<<21)
#define PROC_BASED_PRIMARY_NMI_WINDOW_EXIT                  ((QWORD)1<<22)
#define PROC_BASED_PRIMARY_MOV_DR_EXIT                      ((QWORD)1<<23)
#define PROC_BASED_PRIMARY_UNCONDITIONAL_IO_EXIT            ((QWORD)1<<24)
#define PROC_BASED_PRIMARY_USE_IO_BITMAPS                   ((QWORD)1<<25)
#define PROC_BASED_PRIMARY_MONITOR_TRAP_FLAG                ((QWORD)1<<27)
#define PROC_BASED_PRIMARY_USE_MSR_BITMAPS                  ((QWORD)1<<28)
#define PROC_BASED_PRIMARY_MONITOR_EXIT                     ((QWORD)1<<29)
#define PROC_BASED_PRIMARY_PAUSE_EXIT                       ((QWORD)1<<30)
#define PROC_BASED_PRIMARY_ACTIVATE_SECONDARY_CTLS          ((QWORD)1<<31)


#define VMCS_CONTROL_PRIMARY_PROCBASED_CONTROLS             (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_EXCEPTION_BITMAP                       (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_PF_ERROR_CODE_MASK                     (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_PF_ERROR_CODE_MATCH                    (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_CR3_TARGET_COUNT                       (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 5 << 1 ) | VMCS_ACCESS_FULL )

// 24.7.1 VM-Exit Controls
#define EXIT_CONTROL_SAVE_DEBUG_CTLS                        ((QWORD)1<<2)
#define EXIT_CONTROL_HOST_ADDRESS_SPACE_SIZE                ((QWORD)1<<9)
#define EXIT_CONTROL_LOAD_IA32_PERF_GLOBAL_CTRL             ((QWORD)1<<12)
#define EXIT_CONTROL_ACK_INT_ON_EXIT                        ((QWORD)1<<15)
#define EXIT_CONTROL_SAVE_IA32_PAT                          ((QWORD)1<<18)
#define EXIT_CONTROL_LOAD_IA32_PAT                          ((QWORD)1<<19)
#define EXIT_CONTROL_SAVE_IA32_EFER                         ((QWORD)1<<20)
#define EXIT_CONTROL_LOAD_IA32_EFER                         ((QWORD)1<<21)
#define EXIT_CONTROL_SAVE_VMX_PREEMPT_TIMER_VALUE           ((QWORD)1<<22)

#define VMCS_CONTROL_VM_EXIT_CONTROLS                       (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 6 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_VM_EXIT_MSR_STORE_COUNT                (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 7 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_VM_EXIT_MSR_LOAD_COUNT                 (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 8 << 1 ) | VMCS_ACCESS_FULL )

// 24.8.1 VM-Entry Controls
#define ENTRY_CONTROL_LOAD_DEBUG_CTLS                       ((QWORD)1<<2)
#define ENTRY_CONTROL_IA32E_MODE_GUEST                      ((QWORD)1<<9)
#define ENTRY_CONTROL_ENTRY_TO_SMM                          ((QWORD)1<<10)
#define ENTRY_CONTROL_DEACTIVATE_DUAL_MONITOR               ((QWORD)1<<11)
#define ENTRY_CONTROL_LOAD_IA32_PERF_GLOBAL_CTRL            ((QWORD)1<<13)
#define ENTRY_CONTROL_LOAD_IA32_PAT                         ((QWORD)1<<14)
#define ENTRY_CONTROL_LOAD_IA32_EFER                        ((QWORD)1<<15)

#define VMCS_CONTROL_VM_ENTRY_CONTROLS                      (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 9 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_VM_ENTRY_MSR_LOAD_COUNT                (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 10 << 1 ) | VMCS_ACCESS_FULL )

enum _INTERRUPTION_TYPE
{
    InterruptionTypeExternalInterrupt,
    InterruptionTypeReserved,
    InterruptionTypeNMI,
    InterruptionTypeHardwareException,
    InterruptionTypeSoftwareInterrupt,
    InterruptionTypePrivilegedSoftwareException,
    InterruptionTypeSoftwareException,
    InterruptionTypeOtherEvent
} INTERRUPTION_TYPE;

typedef union _VM_ENTRY_INT_INFO
{
    struct
    {
        DWORD           Vector              :       8;
        DWORD           InterruptionType    :       3;
        DWORD           ErrorCode           :       1;
        DWORD           Reserved            :       19;
        DWORD           Valid               :       1;
    };
    DWORD               Raw;
} VM_ENTRY_INT_INFO, *PVM_ENTRY_INT_INFO;
STATIC_ASSERT( sizeof( VM_ENTRY_INT_INFO ) == sizeof( DWORD ) );

#define VMCS_CONTROL_VM_ENTRY_INT_INFO_FIELD                (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 11 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_VM_ENTRY_EXCEPTION_ERROR_CODE          (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 12 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_VM_ENTRY_INSTRUCTION_LENGTH            (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 13 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_TPR_THRESHOLD                          (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 14 << 1 ) | VMCS_ACCESS_FULL )

// Secondary Processor-Based VM-Execution Controls
#define PROC_BASED_SECONDARY_VIRTUALIZE_APIC_ACCESS         ((QWORD)1<<0)
#define PROC_BASED_SECONDARY_ENABLE_EPT                     ((QWORD)1<<1)
#define PROC_BASED_SECONDARY_DESCRIPTOR_TABLE_EXIT          ((QWORD)1<<2)
#define PROC_BASED_SECONDARY_ENABLE_RDTSCP                  ((QWORD)1<<3)
#define PROC_BASED_SECONDARY_VIRTUALIZE_X2_APIC             ((QWORD)1<<4)
#define PROC_BASED_SECONDARY_ENABLE_VPID                    ((QWORD)1<<5)
#define PROC_BASED_SECONDARY_WBINVD_EXIT                    ((QWORD)1<<6)
#define PROC_BASED_SECONDARY_UNRESTRICTED_GUEST             ((QWORD)1<<7)
#define PROC_BASED_SECONDARY_APIC_REGISTER_VIRTUALIZATION   ((QWORD)1<<8)
#define PROC_BASED_SECONDARY_VIRTUAL_INTERRUPT_DELIVERY     ((QWORD)1<<9)
#define PROC_BASED_SECONDARY_PAUSE_LOOP_EXIT                ((QWORD)1<<10)
#define PROC_BASED_SECONDARY_RDRAND_EXIT                    ((QWORD)1<<11)
#define PROC_BASED_SECONDARY_ENABLE_INVPCID                 ((QWORD)1<<12)
#define PROC_BASED_SECONDARY_ENABLE_VM_FUNCS                ((QWORD)1<<13)
#define PROC_BASED_SECONDARY_VMCS_SHADOWING                 ((QWORD)1<<14)
#define PROC_BASED_SECONDARY_RDSEED_EXIT                    ((QWORD)1<<16)
#define PROC_BASED_SECONDARY_EPT_VIOLATION_INTERRUPT        ((QWORD)1<<18)
#define PROC_BASED_SECONDARY_ENABLE_XSAVES_XSTORS           ((QWORD)1<<20)
#define PROC_BASED_SECONDARY_MODE_BASED_EXECUTE_EPT         ((QWORD)1<<22)
#define PROC_BASED_SECONDARY_USE_TSC_SCALING                ((QWORD)1<<25)

#define VMCS_CONTROL_SECONDARY_PROCBASED_CONTROLS           (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 15 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_PLE_GAP                                (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 16 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_PLE_WINDOW                             (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_CONTROL | ( 17 << 1 ) | VMCS_ACCESS_FULL )

// 32-Bit Exit Fields (B.3.2)

#define VMCS_EXIT_INSTRUCTION_ERROR                         (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_EXIT | ( 0 << 1 ) | VMCS_ACCESS_FULL )

typedef union _EXIT_REASON_STRUCT
{
    struct
    {
        WORD            BasicExitReason;
        WORD            Reserved0                           :   12;
        WORD            PendingMTF                          :   1;
        WORD            RootExit                            :   1;
        WORD            Reserved1                           :   1;
        WORD            EntryFailure                        :   1;
    };
    DWORD               Raw;
} EXIT_REASON_STRUCT, *PEXIT_REASON_STRUCT;
STATIC_ASSERT(sizeof(EXIT_REASON_STRUCT) == sizeof(DWORD));

#define VMCS_EXIT_REASON                                    (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_EXIT | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_INT_INFO                                  (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_EXIT | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_INT_ERROR_CODE                            (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_EXIT | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_IDT_INFO_FIELD                            (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_EXIT | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_IDT_ERROR_CODE                            (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_EXIT | ( 5 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_INSTRUCTION_LENGTH                        (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_EXIT | ( 6 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_INSTRUCTION_INFO                          (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_EXIT | ( 7 << 1 ) | VMCS_ACCESS_FULL )

// 32-Bit Guest-State Fields (B.3.3)

#define VMCS_GUEST_ES_LIMIT                                 (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_CS_LIMIT                                 (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_SS_LIMIT                                 (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_DS_LIMIT                                 (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_FS_LIMIT                                 (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_GS_LIMIT                                 (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 5 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_LDTR_LIMIT                               (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 6 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_TR_LIMIT                                 (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 7 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_GDTR_LIMIT                               (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 8 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_IDTR_LIMIT                               (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 9 << 1 ) | VMCS_ACCESS_FULL )


// Table 24-2 Segment Access Rights useful defines
#define SEGMENT_ACCESS_RIGHTS_DESC_TYPE                     ((QWORD)1<<4)
#define SEGMENT_ACCESS_RIGHTS_P_BIT                         ((QWORD)1<<7)
#define SEGMENT_ACCESS_RIGHTS_L_BIT                         ((QWORD)1<<13)
#define SEGMENT_ACCESS_RIGHTS_DB_BIT                        ((QWORD)1<<14)
#define SEGMENT_ACCESS_RIGHTS_G_BIT                         ((QWORD)1<<15)
#define SEGMENT_ACCESS_RIGHTS_UNUSABLE                      ((QWORD)1<<16)

#define VMCS_GUEST_ES_ACCESS_RIGHTS                         (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 10 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_CS_ACCESS_RIGHTS                         (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 11 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_SS_ACCESS_RIGHTS                         (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 12 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_DS_ACCESS_RIGHTS                         (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 13 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_FS_ACCESS_RIGHTS                         (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 14 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_GS_ACCESS_RIGHTS                         (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 15 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_LDTR_ACCESS_RIGHTS                       (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 16 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_TR_ACCESS_RIGHTS                         (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 17 << 1 ) | VMCS_ACCESS_FULL )

#define INT_STATE_BLOCKING_BY_STI                           ((QWORD)1<<0)
#define INT_STATE_BLOCKING_BY_MOV_SS                        ((QWORD)1<<1)

#define VMCS_GUEST_INT_STATE                                (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 18 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_ACTIVITY_STATE                           (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 19 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_SMBASE                                   (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 20 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_IA32_SYSENTER_CS                         (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 21 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_VMX_PREEMPT_TIMER_VALUE                  (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_GUEST | ( 23 << 1 ) | VMCS_ACCESS_FULL )

// 32-Bit Host-State Fields (B.3.4)

#define VMCS_HOST_IA32_SYSENTER_CS                          (VMCS_FIELD)( VMCS_FIELD_32_BIT | VMCS_TYPE_HOST | ( 0 << 1 ) | VMCS_ACCESS_FULL )

// Natural-Width Fields (B.4)

// Natural-Width Control Fields (B.4.1)

#define VMCS_CONTROL_CR0_MASK                               (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_CONTROL | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_CR4_MASK                               (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_CONTROL | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_CR0_READ_SHADOW                        (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_CONTROL | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_CR4_READ_SHADOW                        (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_CONTROL | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_CR3_TARGET_0                           (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_CONTROL | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_CR3_TARGET_1                           (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_CONTROL | ( 5 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_CR3_TARGET_2                           (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_CONTROL | ( 6 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_CONTROL_CR3_TARGET_3                           (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_CONTROL | ( 7 << 1 ) | VMCS_ACCESS_FULL )

// Natural-Width Exit Fields (B.4.2)

#define VMCS_EXIT_QUALIFICATION                             (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_EXIT | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_IO_RCX                                    (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_EXIT | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_IO_RSI                                    (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_EXIT | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_IO_RDI                                    (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_EXIT | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_IO_RIP                                    (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_EXIT | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_EXIT_GUEST_LINEAR_ADDRESS                      (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_EXIT | ( 5 << 1 ) | VMCS_ACCESS_FULL )

// Natural-Width Guest-State Fields (B.4.3)

#define VMCS_GUEST_CR0                                      (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_CR3                                      (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_CR4                                      (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_ES_BASE                                  (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_CS_BASE                                  (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_SS_BASE                                  (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 5 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_DS_BASE                                  (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 6 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_FS_BASE                                  (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 7 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_GS_BASE                                  (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 8 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_LDTR_BASE                                (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 9 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_TR_BASE                                  (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 10 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_GDTR_BASE                                (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 11 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_IDTR_BASE                                (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 12 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_DR7                                      (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 13 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_RSP                                      (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 14 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_RIP                                      (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 15 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_RFLAGS                                   (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 16 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS                 (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 17 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_IA32_SYSENTER_ESP                        (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 18 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_GUEST_IA32_SYSENTER_EIP                        (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_GUEST | ( 19 << 1 ) | VMCS_ACCESS_FULL )

// Natural-Width Host-State Fields (B.4.4)

#define VMCS_HOST_CR0                                       (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 0 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_CR3                                       (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 1 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_CR4                                       (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 2 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_FS_BASE                                   (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 3 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_GS_BASE                                   (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 4 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_TR_BASE                                   (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 5 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_GDTR_BASE                                 (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 6 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_IDTR_BASE                                 (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 7 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_IA32_SYSENTER_ESP                         (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 8 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_IA32_SYSENTER_EIP                         (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 9 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_RSP                                       (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 10 << 1 ) | VMCS_ACCESS_FULL )
#define VMCS_HOST_RIP                                       (VMCS_FIELD)( VMCS_FIELD_NATURAL | VMCS_TYPE_HOST | ( 11 << 1 ) | VMCS_ACCESS_FULL )

#pragma warning(pop)
#pragma pack(pop)
