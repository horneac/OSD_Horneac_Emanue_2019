#pragma once

#include "cl_heap.h"
#include "heap_tags.h"

typedef struct _SYSTEM_INFORMATION
{
    QWORD                           SystemUptimeUs;
    QWORD                           CpuFrequency;
    QWORD                           TotalPhysicalMemory;
    PHYSICAL_ADDRESS                HighestPhysicalAddress;
} SYSTEM_INFORMATION, *PSYSTEM_INFORMATION;

_Always_(_When_(IsBooleanFlagOn(Flags, PoolAllocatePanicIfFail), RET_NOT_NULL))
PTR_SUCCESS
PVOID
ExAllocatePoolWithTag(
    IN      DWORD                   Flags,
    IN      DWORD                   AllocationSize,
    IN      DWORD                   Tag,
    IN      DWORD                   AllocationAlignment
    );

void
ExFreePoolWithTag(
    _Pre_notnull_ _Post_ptr_invalid_
            PVOID                   MemoryAddress,
    IN      DWORD                   Tag
    );

void
ExGetSystemInformation(
    OUT     PSYSTEM_INFORMATION     SystemInformation
    );