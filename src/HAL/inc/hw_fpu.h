#pragma once

#define INCLUDE_FP_SUPPORT                  0

typedef QWORD XCR0_SAVED_STATE;

#define XCR0_SAVED_STATE_x87_MMX            (1ULL<<0)
#define XCR0_SAVED_STATE_SSE                (1ULL<<1)
#define XCR0_SAVED_STATE_AVX                (1ULL<<2)
#define XCR0_SAVED_STATE_BNDREG             (1ULL<<3)
#define XCR0_SAVED_STATE_BNDCSR             (1ULL<<4)
#define XCR0_SAVED_STATE_OPMASK             (1ULL<<5)
#define XCR0_SAVED_STATE_ZMM_HI256          (1ULL<<6)
#define XCR0_SAVED_STATE_HI16_ZMM           (1ULL<<7)
#define XCR0_SAVED_STATE_PKRU               (1ULL<<9)

#pragma pack(push)
#pragma pack(1)

#pragma warning(push)

// warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable:4201)
typedef struct _M128A
{
    QWORD                   Low;
    QWORD                   High;
} M128A, *PM128A;

//
// Format of data for (F)XSAVE/(F)XRSTOR instruction
//
#define PREDEFINED_XSAVE_LEGACY_REGION_SIZE         0x200

typedef struct  _XSAVE_LEGACY_REGION
{
    WORD                        ControlWord;
    WORD                        StatusWord;
    BYTE                        TagWord;
    BYTE                        Reserved1;
    WORD                        ErrorOpcode;
    DWORD                       ErrorOffset;
    WORD                        ErrorSelector;
    WORD                        Reserved2;
    DWORD                       DataOffset;
    WORD                        DataSelector;
    WORD                        Reserved3;
    DWORD                       MxCsr;
    DWORD                       MxCsr_Mask;
    M128A                       FloatRegisters[8];
    M128A                       XmmRegisters[16];
    BYTE                        Reserved4[96];
} XSAVE_LEGACY_REGION, *PXSAVE_LEGACY_REGION;
STATIC_ASSERT_INFO(sizeof(XSAVE_LEGACY_REGION) == PREDEFINED_XSAVE_LEGACY_REGION_SIZE,
    "Intel Software Developer Manual Vol 1 Section 13.4.1 Legacy Region of an XSAVE Area");

#define PREDEFINED_XSAVE_AREA_HEADER_SIZE           0x40

typedef struct  _XSAVE_AREA_HEADER
{
    QWORD                       Mask;
    QWORD                       Reserved[7];
} XSAVE_AREA_HEADER, *PXSAVE_AREA_HEADER;
STATIC_ASSERT_INFO(sizeof(XSAVE_AREA_HEADER) == PREDEFINED_XSAVE_AREA_HEADER_SIZE,
    "Intel Software Developer Manual Vol 1 Section 13.4.2 XSAVE Header");

#define HAL_MAX_SUPPORTED_XSAVE_AREA_SIZE           0x3C0
#define XSAVE_AREA_REQUIRED_ALIGNMENT               0x40

#define HAL_XSAVE_AREA_RESERVED_SIZE                (HAL_MAX_SUPPORTED_XSAVE_AREA_SIZE+XSAVE_AREA_REQUIRED_ALIGNMENT)

typedef union _XSAVE_AREA
{
    struct
    {
        XSAVE_LEGACY_REGION     LegacyState;
        XSAVE_AREA_HEADER       Header;
    };
    BYTE                        __Reserved[HAL_XSAVE_AREA_RESERVED_SIZE];
} XSAVE_AREA, *PXSAVE_AREA;
STATIC_ASSERT(sizeof(XSAVE_AREA) == HAL_XSAVE_AREA_RESERVED_SIZE);

#pragma warning(pop)
#pragma pack(pop)

// This function is expected to be called very, very early, it does not
// call any other function and has optimizations turned off explicitly
// to make sure the compiler doesn't generate SSE instructions before
// actually activating SSE support :)
void
HalActivateFpu(
    void
    );

// This function must be called after HalActivateFpu
STATUS
HalSetActiveFpuFeatures(
    _In_        XCR0_SAVED_STATE            Features
    );

XCR0_SAVED_STATE
HalGetActiveFpuFeatures(
    _Out_opt_   DWORD*                      ActivatedFeaturesSaveSize,
    _Out_opt_   DWORD*                      AvailableFeaturesSaveSize
    );
