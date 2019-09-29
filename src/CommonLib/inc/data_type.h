#pragma once

C_HEADER_START

#pragma warning(push)

// warning C4142: 'DWORD': benign redefinition of type
#pragma warning(disable:4142)

#ifndef TRUE
#define TRUE                        ( 1 == 1 )
#endif

#ifndef FALSE
#define FALSE                       ( 1 == 0 )
#endif

#ifndef NULL
#define NULL                        ( (void*) 0 )
#endif

#define MAX_NIBBLE                  0xFU
#define MAX_BYTE                    0xFFU
#define MAX_WORD                    0xFFFFU
#define MAX_DWORD                   0xFFFFFFFFUL
#define MAX_QWORD                   0xFFFFFFFFFFFFFFFFULL

//
// standard types - define them with explicit length
//
typedef unsigned __int8     BYTE, *PBYTE;
typedef unsigned __int16    WORD, *PWORD;
typedef unsigned __int32    DWORD, *PDWORD;
typedef unsigned __int64    QWORD, *PQWORD;

typedef unsigned __int8     UINT8, *PUINT8;
typedef unsigned __int16    UINT16, *PUINT16;
typedef unsigned __int32    UINT32, *PUINT32;
typedef unsigned __int64    UINT64, *PUINT64;

typedef signed __int8       INT8;
typedef signed __int16      INT16;
typedef signed __int32      INT32;
typedef signed __int64      INT64;

// pointer
typedef void*               PVOID;

// bool
typedef BYTE                BOOLEAN;

// VMX operation
typedef BYTE                VMX_RESULT;

#ifndef CL_DO_NOT_DEFINE_PHYSICAL_ADDRESS
// physical memory address
typedef PVOID               PHYSICAL_ADDRESS;
#endif // CL_DO_NOT_DEFINE_PHYSICAL_ADDRESS

typedef volatile BYTE       VOL_BYTE;
typedef volatile WORD       VOL_WORD;
typedef volatile DWORD      VOL_DWORD;
typedef volatile QWORD      VOL_QWORD;

#ifndef _WCHAR_T_DEFINED
typedef unsigned short WCHAR;
#define _WCHAR_T_DEFINED
#endif  /* _WCHAR_T_DEFINED */

#pragma warning(pop)

C_HEADER_END
