#pragma once

#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)

typedef unsigned __int64    ULONGLONG, DWORD64;
typedef __int64             LONGLONG;
typedef unsigned __int32    ULONG32;

typedef enum _EXCEPTION_DISPOSITION
{
    ExceptionContinueExecution,
    ExceptionContinueSearch,
    ExceptionNestedException,
    ExceptionCollidedUnwind
} EXCEPTION_DISPOSITION;

typedef struct _EXCEPTION_RECORD         // 7 elements, 0x98 bytes (sizeof)
{
    /*0x000*/     INT32       ExceptionCode;
    /*0x004*/     DWORD      ExceptionFlags;
    /*0x008*/     QWORD       ExceptionRecord;
    /*0x010*/     QWORD       ExceptionAddress;
    /*0x018*/     DWORD      NumberParameters;
    /*0x01C*/     DWORD      __unusedAlignment;
    /*0x020*/     QWORD       ExceptionInformation[15];
}EXCEPTION_RECORD, *PEXCEPTION_RECORD;

typedef struct __declspec(align(16)) _M128A
{
    ULONGLONG Low;
    LONGLONG High;
} M128A, *PM128A;

typedef struct __declspec(align(16)) _XSAVE_FORMAT
{
    WORD   ControlWord;
    WORD   StatusWord;
    BYTE  TagWord;
    BYTE  Reserved1;
    WORD   ErrorOpcode;
    DWORD ErrorOffset;
    WORD   ErrorSelector;
    WORD   Reserved2;
    DWORD DataOffset;
    WORD   DataSelector;
    WORD   Reserved3;
    DWORD MxCsr;
    DWORD MxCsr_Mask;
    M128A FloatRegisters[8];
    M128A XmmRegisters[16];
    BYTE  Reserved4[96];
} XSAVE_FORMAT, *PXSAVE_FORMAT;





typedef struct _CONTEXT                    // 64 elements, 0x4D0 bytes (sizeof)
{
    /*0x000*/     UINT64       P1Home;
    /*0x008*/     UINT64       P2Home;
    /*0x010*/     UINT64       P3Home;
    /*0x018*/     UINT64       P4Home;
    /*0x020*/     UINT64       P5Home;
    /*0x028*/     UINT64       P6Home;
    /*0x030*/     ULONG32      ContextFlags;
    /*0x034*/     ULONG32      MxCsr;
    /*0x038*/     UINT16       SegCs;
    /*0x03A*/     UINT16       SegDs;
    /*0x03C*/     UINT16       SegEs;
    /*0x03E*/     UINT16       SegFs;
    /*0x040*/     UINT16       SegGs;
    /*0x042*/     UINT16       SegSs;
    /*0x044*/     ULONG32      EFlags;
    /*0x048*/     UINT64       Dr0;
    /*0x050*/     UINT64       Dr1;
    /*0x058*/     UINT64       Dr2;
    /*0x060*/     UINT64       Dr3;
    /*0x068*/     UINT64       Dr6;
    /*0x070*/     UINT64       Dr7;
    /*0x078*/     UINT64       Rax;
    /*0x080*/     UINT64       Rcx;
    /*0x088*/     UINT64       Rdx;
    /*0x090*/     UINT64       Rbx;
    /*0x098*/     UINT64       Rsp;
    /*0x0A0*/     UINT64       Rbp;
    /*0x0A8*/     UINT64       Rsi;
    /*0x0B0*/     UINT64       Rdi;
    /*0x0B8*/     UINT64       R8;
    /*0x0C0*/     UINT64       R9;
    /*0x0C8*/     UINT64       R10;
    /*0x0D0*/     UINT64       R11;
    /*0x0D8*/     UINT64       R12;
    /*0x0E0*/     UINT64       R13;
    /*0x0E8*/     UINT64       R14;
    /*0x0F0*/     UINT64       R15;
    /*0x0F8*/     UINT64       Rip;
    union                                  // 2 elements, 0x200 bytes (sizeof)
    {
        /*0x100*/         struct _XSAVE_FORMAT FltSave;      // 16 elements, 0x200 bytes (sizeof)
        struct                             // 18 elements, 0x200 bytes (sizeof)
        {
            /*0x100*/             struct _M128A Header[2];
            /*0x120*/             struct _M128A Legacy[8];
            /*0x1A0*/             struct _M128A Xmm0;            // 2 elements, 0x10 bytes (sizeof)
            /*0x1B0*/             struct _M128A Xmm1;            // 2 elements, 0x10 bytes (sizeof)
            /*0x1C0*/             struct _M128A Xmm2;            // 2 elements, 0x10 bytes (sizeof)
            /*0x1D0*/             struct _M128A Xmm3;            // 2 elements, 0x10 bytes (sizeof)
            /*0x1E0*/             struct _M128A Xmm4;            // 2 elements, 0x10 bytes (sizeof)
            /*0x1F0*/             struct _M128A Xmm5;            // 2 elements, 0x10 bytes (sizeof)
            /*0x200*/             struct _M128A Xmm6;            // 2 elements, 0x10 bytes (sizeof)
            /*0x210*/             struct _M128A Xmm7;            // 2 elements, 0x10 bytes (sizeof)
            /*0x220*/             struct _M128A Xmm8;            // 2 elements, 0x10 bytes (sizeof)
            /*0x230*/             struct _M128A Xmm9;            // 2 elements, 0x10 bytes (sizeof)
            /*0x240*/             struct _M128A Xmm10;           // 2 elements, 0x10 bytes (sizeof)
            /*0x250*/             struct _M128A Xmm11;           // 2 elements, 0x10 bytes (sizeof)
            /*0x260*/             struct _M128A Xmm12;           // 2 elements, 0x10 bytes (sizeof)
            /*0x270*/             struct _M128A Xmm13;           // 2 elements, 0x10 bytes (sizeof)
            /*0x280*/             struct _M128A Xmm14;           // 2 elements, 0x10 bytes (sizeof)
            /*0x290*/             struct _M128A Xmm15;           // 2 elements, 0x10 bytes (sizeof)
            /*0x2A0*/             UINT8        _PADDING0_[0x60];
        };
    };
    /*0x300*/     struct _M128A VectorRegister[26];
    /*0x4A0*/     UINT64       VectorControl;
    /*0x4A8*/     UINT64       DebugControl;
    /*0x4B0*/     UINT64       LastBranchToRip;
    /*0x4B8*/     UINT64       LastBranchFromRip;
    /*0x4C0*/     UINT64       LastExceptionToRip;
    /*0x4C8*/     UINT64       LastExceptionFromRip;
}CONTEXT, *PCONTEXT;

typedef
EXCEPTION_DISPOSITION
EXCEPTION_ROUTINE(
    _Inout_ struct _EXCEPTION_RECORD *ExceptionRecord,
    _In_ PVOID EstablisherFrame,
    _Inout_ struct _CONTEXT *ContextRecord,
    _In_ PVOID DispatcherContext
);

typedef EXCEPTION_ROUTINE *PEXCEPTION_ROUTINE;

typedef struct _RUNTIME_FUNCTION
{
    // RVAs
    DWORD BeginAddress;
    DWORD EndAddress;
    DWORD UnwindInfoAddress;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

typedef struct _UNWIND_HISTORY_TABLE_ENTRY
{
    DWORD64 ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
} UNWIND_HISTORY_TABLE_ENTRY, *PUNWIND_HISTORY_TABLE_ENTRY;

#define UNWIND_HISTORY_TABLE_SIZE 12

typedef struct _UNWIND_HISTORY_TABLE
{
    DWORD Count;
    BYTE  LocalHint;
    BYTE  GlobalHint;
    BYTE  Search;
    BYTE  Once;
    DWORD64 LowAddress;
    DWORD64 HighAddress;
    UNWIND_HISTORY_TABLE_ENTRY Entry[UNWIND_HISTORY_TABLE_SIZE];
} UNWIND_HISTORY_TABLE, *PUNWIND_HISTORY_TABLE;

typedef struct _DISPATCHER_CONTEXT
{
    DWORD64 ControlPc;
    DWORD64 ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
    DWORD64 EstablisherFrame;
    DWORD64 TargetIp;
    PCONTEXT ContextRecord;
    PEXCEPTION_ROUTINE LanguageHandler;
    PVOID HandlerData;
    PUNWIND_HISTORY_TABLE HistoryTable;
    DWORD ScopeIndex;
    DWORD Fill0;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;

#pragma warning(pop)
