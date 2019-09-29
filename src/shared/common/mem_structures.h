#pragma once

typedef DWORD                       PAGE_RIGHTS;

#define PAGE_RIGHTS_READ            0
#define PAGE_RIGHTS_WRITE           1
#define PAGE_RIGHTS_EXECUTE         2

#define PAGE_RIGHTS_READWRITE       (PAGE_RIGHTS_READ | PAGE_RIGHTS_WRITE )
#define PAGE_RIGHTS_READEXEC        (PAGE_RIGHTS_READ | PAGE_RIGHTS_EXECUTE )
#define PAGE_RIGHTS_ALL             (PAGE_RIGHTS_READ | PAGE_RIGHTS_WRITE | PAGE_RIGHTS_EXECUTE )

typedef DWORD                       VMM_ALLOC_TYPE;

#define VMM_ALLOC_TYPE_RESERVE      0x1
#define VMM_ALLOC_TYPE_COMMIT       0x2
#define VMM_ALLOC_TYPE_NOT_LAZY     0x4
#define VMM_ALLOC_TYPE_ZERO         0x8

typedef DWORD                       VMM_FREE_TYPE;

#define VMM_FREE_TYPE_DECOMMIT      0x1
#define VMM_FREE_TYPE_RELEASE       0x2
