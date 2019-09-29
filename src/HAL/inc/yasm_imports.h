#pragma once

#include "gdt.h"

typedef
void
(__cdecl FUNC_Magic) (
    void
    );

//******************************************************************************
// Function:     FUNC_Magic
// Description:  Generates bochs magic breakpoint
// Returns:      void
//******************************************************************************
extern FUNC_Magic       __magic;

typedef
void
(__cdecl FUNC_StiAndHlt) (
   void
   );

//******************************************************************************
// Function:     __sti_and_hlt
// Description:  Enables interrupts and halts
// Returns:      void
//******************************************************************************
extern FUNC_StiAndHlt   __sti_and_hlt;

typedef 
void
(__cdecl FUNC_LoadTR ) (
    IN       WORD       GdtIndex               
    );

//******************************************************************************
// Function:      __ltr
// Description:   Loads a new TR selector.
// Returns:       void
// Parameter:     IN WORD GdtIndex - index in GDT of the descriptor
//******************************************************************************
extern FUNC_LoadTR      __ltr;

typedef
void
(__cdecl FUNC_StoreGDT) (
    OUT     PGDT        Gdt
    );

//******************************************************************************
// Function:      __ltr
// Description:   Loads a new TR selector.
// Returns:       void
// Parameter:     IN WORD GdtIndex - index in GDT of the descriptor
//******************************************************************************
extern FUNC_StoreGDT    __sgdt;

typedef
void
(__cdecl FUNC_ChangeStack)(
    IN  PVOID       InitialStackBase,
    IN  PVOID       NewStackBase
    );

extern FUNC_ChangeStack __changeStack;