#pragma once

C_HEADER_START
//******************************************************************************
// Function:     FUNC_FreeFunction
// Description:  Function called when the ReferenceCount of the object reaches 0
//               This function should free any allocated resources or handles.
// Returns:      void
// Parameter:    _cdecl FUNC_FreeFunction
//******************************************************************************
typedef
void
(_cdecl FUNC_FreeFunction)(
    IN      PVOID       Object,
    IN_OPT  PVOID       Context
    );

typedef FUNC_FreeFunction*      PFUNC_FreeFunction;

#pragma pack(push,16)
typedef struct _REF_COUNT
{
    volatile DWORD              ReferenceCount;
    PFUNC_FreeFunction          FreeFunction;
    PVOID                       Context;
} REF_COUNT, *PREF_COUNT;
#pragma pack(pop)

void
RfcPreInit(
    OUT     REF_COUNT*              Object
    );

//******************************************************************************
// Function:     RfcInit
// Description:  Initializes a reference counted object.
// Returns:      STATUS
// Parameter:    OUT REF_COUNT * Object
// Parameter:    IN_OPT PFUNC_FreeFunction FreeFunction - Called when reference
//               count for object reaches 0.
// Parameter:    IN_OPT PVOID Context - Context to pass to FreeFunction
//******************************************************************************
STATUS
RfcInit(
    OUT     REF_COUNT*              Object,
    IN_OPT  PFUNC_FreeFunction      FreeFunction,
    IN_OPT  PVOID                   Context
    );

//******************************************************************************
// Function:     RfcReference
// Description:  Increments the reference count of the object
// Returns:      DWORD - Current reference count of the object
// Parameter:    INOUT REF_COUNT * Object
//******************************************************************************
SIZE_SUCCESS
DWORD
RfcReference(
    INOUT   REF_COUNT*              Object
    );

//******************************************************************************
// Function:     RfcReference
// Description:  Decrements the reference count of the object
// Returns:      DWORD - Current reference count of the object
// Parameter:    INOUT REF_COUNT * Object
//******************************************************************************
SIZE_SUCCESS
DWORD
RfcDereference(
    INOUT   REF_COUNT*              Object
    );
C_HEADER_END
