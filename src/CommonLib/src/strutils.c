#include "common_lib.h"
#include "strutils.h"

__forceinline
void
swap(
    INOUT   BYTE* a,
    INOUT   BYTE* b
    )
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

void
itoa(
    IN      PVOID       valueAddress,
    IN      BOOLEAN     signedValue,
    OUT_Z   char*       buffer,
    IN      DWORD       base,
    IN      BOOLEAN     is64BitValue
    )
{
    DWORD index = 0;
    DWORD i;
    QWORD value;
    BOOLEAN negative;

    ASSERT( NULL != valueAddress );
    ASSERT( NULL != buffer );
    ASSERT( 0 != base );

    negative = FALSE;

    if (is64BitValue)
    {
        if (signedValue)
        {
            if (0 > *(INT64*)valueAddress)
            {
                negative = TRUE;
            }
        }

        if (negative)
        {
            value = (-(*(INT64*)valueAddress));
        }
        else
        {
            value = *(QWORD*)valueAddress;
        }
    }
    else
    {
        if (signedValue)
        {
            if (0 > *(INT32*)valueAddress)
            {
                negative = TRUE;
            }
        }

        if (negative)
        {
            value = (-(*(INT32*)valueAddress));
        }
        else
        {
            value = *(DWORD*)valueAddress;
        }
    }



    if (0 == value)
    {
        buffer[index] = '0';
        index++;
    }

    // we convert the number and get a
    // reversed ordered string
    while (0 != value)
    {
        // we get the current digit
        int digit = (value % base);

        // we convert it to an ASCII character
        if (digit > 9)
        {
            buffer[index] = (char)(digit - 10) + 'A';
        }
        else
        {
            buffer[index] = (char)digit + '0';
        }

        value = value / base;
        index++;
    }

    if (negative)
    {
        buffer[index] = '-';
        index++;
    }

    // we null terminate the string
    buffer[index] = '\0';

    for (i = 0; i < index / 2; ++i)
    {
        // we reverse the string to have it ordered right
        swap((BYTE*)&buffer[i], (BYTE*)&buffer[index - i - 1]);
    }
}

void
atoi(
    _When_(!is64BitValue, OUT_WRITES_BYTES_ALL(sizeof(DWORD)))
    _When_(is64BitValue, OUT_WRITES_BYTES_ALL(sizeof(QWORD)))
            PVOID       valueAddress,
    IN_Z    char*       buffer,
    IN      DWORD       base,
    IN      BOOLEAN     is64BitValue
    )
{
    DWORD i;
    QWORD value;
    BOOLEAN negative;

    ASSERT( NULL != valueAddress );
    ASSERT( NULL != buffer );
    ASSERT( 0 != base );

    i = 0;
    value = 0;
    negative = FALSE;

    if ('-' == buffer[0])
    {
        negative = TRUE;
        i = 1;
    }

    for (; buffer[i] != '\0'; ++i)
    {
        char c;
        BYTE currentCharValue;

        ASSERT(isascii(buffer[i]));

        // it's ok to use tolower even if we have a digit
        // because they are 0x30 -> 0x39 and an OR with 0x20 leaves
        // them unmodified
        c = tolower(buffer[i]);

        if ('0' <= c && c <= '9')
        {
            currentCharValue = c - '0';
        }
        else if ('a' <= c && c <= 'z')
        {
            currentCharValue = c - 'a' + 10;
        }
        else
        {
            currentCharValue = MAX_BYTE;
        }

        ASSERT( currentCharValue < base );

        // digit
        value = value * base + currentCharValue;
    }

    value = negative ? - (INT64)value : value;

    if (is64BitValue)
    {
        *((PQWORD)valueAddress) = value;
    }
    else
    {
        // we can either have any negative number
        // or positive numbers smaller or equal to MAX_DWORD
        ASSERT( negative || (value <= MAX_DWORD) );

        *((PDWORD)valueAddress) = (DWORD) value;
    }
}