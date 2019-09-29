#include "ut_base.h"
#include "ut_cl_string.h"

#define MAX_STRING_LENGTH           100

typedef enum _UT_STRING_TESTED_FUNCS
{
    UtStringTrim = 0,

    UtStringReserved = UtStringTrim + 1
} UT_STRING_TESTED_FUNCS;

typedef
BOOLEAN
(__cdecl FUNC_UtStringTestFunc)(
    _In_z_  const char*     OriginalString,
    _In_z_  const char*     ExpectedResult
    );

typedef FUNC_UtStringTestFunc *PFUNC_UtStringTestFunc;

#pragma warning(push)

// warning C4201 : nonstandard extension used : nameless struct / union
#pragma warning(disable:4201)

// warning C4200 : nonstandard extension used : zero - sized array in struct / union
#pragma warning(disable:4200)

typedef struct _UT_SINGLE_STRING_TEST
{
    char                        OriginalString[MAX_STRING_LENGTH];

    union
    {
        struct
        {
            char                ExpectedTrimResult[MAX_STRING_LENGTH];
        };

        char                    ExpectedResults[UtStringReserved][MAX_STRING_LENGTH];
    };

} UT_SINGLE_STRING_TEST;

typedef struct _UT_STRING_TC_ARGS
{
    PFUNC_UtStringTestFunc      Functions[UtStringReserved];
    DWORD                       NumberOfFunctions;

    DWORD                       NumberOfStrings;

    const UT_SINGLE_STRING_TEST *Strings;
} UT_STRING_TC_ARGS;
#pragma warning(pop)

static FUNC_UtStringTestFunc _UtStringTestTrim;

static const UT_SINGLE_STRING_TEST STR_LIST_TO_TEST[] =
{
    {"Test Simplu", "Test Simplu"},
    {"",  ""},
    {"\0",  "\0"},
    {"\03",  "\03"},
    {"   TEST SIMPLu",  "TEST SIMPLu"},
    {"           ",  ""},
    {" AAAAa     ",  "AAAAa"},
    { "      A",  "A"},
    {"B      ", "B"}
};


static const UT_STRING_TC_ARGS TC_ARGS =
{
    _UtStringTestTrim,
    UtStringReserved,

    ARRAYSIZE(STR_LIST_TO_TEST),
    STR_LIST_TO_TEST
};


STATUS
UtClStrings(
    void
    )
{
    for (DWORD i = 0; i < TC_ARGS.NumberOfStrings; ++i)
    {
        for (DWORD j = 0; j < TC_ARGS.NumberOfFunctions; ++j)
        {
            if (!(TC_ARGS.Functions[j])(
                TC_ARGS.Strings[i].OriginalString,
                TC_ARGS.Strings[i].ExpectedResults[j]))
            {
                LOG_ERROR("String %u [%s] was not processed correctly by function %u!\n",
                    i, TC_ARGS.Strings[i].OriginalString, j);
            }
        }


    }

    return CL_STATUS_SUCCESS;
}

static
BOOLEAN
(__cdecl _UtStringTestTrim)(
    _In_z_  const char*     OriginalString,
    _In_z_  const char*     ExpectedResult
    )
{
    char tempString[MAX_STRING_LENGTH];

    cl_strcpy(tempString, OriginalString);
    cl_strtrim(tempString);

    BOOLEAN bSuccess = (cl_strcmp(tempString, ExpectedResult) == 0);

    if (!bSuccess)
    {
        LOG_ERROR("Entry was not trimmed correctly. Actual result [%s] vs expected [%s]. Original string was [%s]\n",
            tempString, ExpectedResult,OriginalString);
    }

    return bSuccess;
}
