#pragma once

void
SerialInitialize(
    IN  WORD    Port
    );

//******************************************************************************
// Function:    SerialWriteBuffer
// Description:
// Returns:       void
// Parameter:     IN char * Buffer
//******************************************************************************
void 
SerialWriteBuffer( 
    IN   WORD   Port,
    IN_Z char*  Buffer 
    );


//******************************************************************************
// Function:    SerialWriteNBuffer
// Description:
// Returns:       void
// Parameter:     IN BYTE * Buffer
// Parameter:     IN DWORD BufferLength
//******************************************************************************
void 
SerialWriteNBuffer( 
    IN                              WORD  Port,
    IN_READS_BYTES(BufferLength)    BYTE* Buffer, 
    IN                              DWORD BufferLength 
    );