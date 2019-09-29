#pragma once

C_HEADER_START
// most often used numbering bases
#define BASE_TWO            2
#define BASE_TEN            10
#define BASE_HEXA           16

#define isascii(c)          ( ( 0x1F < (c)) && ((c) < 0x80) )
#define isupper(c)          ( ( 'A' <= (c)) && ((c) <= 'Z' ) )
#define islower(c)          ( ( 'a' <= (c)) && ((c) <= 'z'))
#define tolower(c)          ( (c) | LOWER_UPPER_DIFF)
#define toupper(c)          ( (c) & (~LOWER_UPPER_DIFF))
#define isspace(c)          ((' ' == (c)) || ('\t' == (c)) || ('\n' == (c)) || ('\r' == (c)))

#define LOWER_UPPER_DIFF    ('a' - 'A')

//******************************************************************************
// Function:      itoa
// Description:   Converts a number into its string representation from the
//                specified base.
//                If the number digits occupied by value in base Base is under
//                MinimumDigits then the rest is completed with leading zeros.
// Returns:       void
// Parameter:     IN PVOID valueAddress - Pointer to the number to convert
// Parameter:     IN BOOLEAN signedValue - If set the value is signed, else unsigned
// Parameter:     OUT char * buffer - Buffer in which to write the number
// Parameter:     IN DWORD base - Numeric base of the input value
// Parameter:     IN DWORD minimumDigits - The minimum number of digits to place
//                in the buffer
// Parameter:     IN BOOLEAN is64BitValue - If set the value is treated as a 64bit
//                value
//******************************************************************************
void
itoa(
    IN      PVOID       valueAddress,
    IN      BOOLEAN     signedValue,
    OUT_Z   char*       buffer,
    IN      DWORD       base,
    IN      BOOLEAN     is64BitValue
    );

void
atoi(
    _When_(!is64BitValue, OUT_WRITES_BYTES_ALL(sizeof(DWORD)))
    _When_(is64BitValue, OUT_WRITES_BYTES_ALL(sizeof(QWORD)))
            PVOID       valueAddress,
    IN_Z    char*       buffer,
    IN      DWORD       base,
    IN      BOOLEAN     is64BitValue
    );

#define atoi32(addr,buf,base)       atoi((addr),(buf),(base),FALSE)
#define atoi64(addr,buf,base)       atoi((addr),(buf),(base),TRUE)
C_HEADER_END
