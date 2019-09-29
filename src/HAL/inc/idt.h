#pragma once


//******************************************************************************
// Function:     IdtInitialize
// Description:  Initializes the IDT with empty entries and loads the IDT for
//               system use.
// Returns:      void
// Parameter:    void
//******************************************************************************
void
IdtInitialize(
    void
    );

//******************************************************************************
// Function:     IdtInstallDescriptor
// Description:  Install a descriptor in the IDT table.
// Returns:      STATUS
// Parameter:    IN BYTE InterruptIndex - Index at which to install interrupt
// Parameter:    IN WORD CodeSelector
// Parameter:    IN BYTE GateType
// Parameter:    IN BYTE InterruptStackIndex
// Parameter:    IN BOOLEAN Present
// Parameter:    IN PVOID HandlerAddress - Address of interrupt handler
//******************************************************************************
STATUS
IdtInstallDescriptor(
    IN                  BYTE            InterruptIndex,
    IN                  WORD            CodeSelector,
    IN                  BYTE            GateType,
    IN                  BYTE            InterruptStackIndex,
    IN                  BOOLEAN         Present,
    _When_(Present, IN) 
    _When_(!Present,IN_OPT) 
    PVOID           HandlerAddress
    );

void
IdtReload(
    void
    );