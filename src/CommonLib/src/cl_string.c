#include "common_lib.h"
#include "cl_string.h"
#include "strutils.h"

// 64 characters needed in case of %B specifier
// with NULL terminator => 65 characters are required
#define VSNPRINTF_BUFFER_SIZE               65

int
cl_strcmp(
    IN_Z  char* str1,
    IN_Z  char* str2
    )
{
    DWORD i;

    if (NULL == str1)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == str2)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    i = 0;

    while (('\0' != str1[i]) && ('\0' != str2[i]))
    {
        if (str1[i] > str2[i])
        {
            return 1;
        }

        if (str1[i] < str2[i])
        {
            return -1;
        }

        ++i;
    }

    // it means the second string is over but the first still has
    // some characters
    if ('\0' != str1[i])
    {
        return 1;
    }

    if ('\0' != str2[i])
    {
        return -1;
    }

    return 0;
}

int
cl_stricmp(
    IN_Z  char* str1,
    IN_Z  char* str2
    )
{
    DWORD i;

    if (NULL == str1)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == str2)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    i = 0;

    while (('\0' != str1[i]) && ('\0' != str2[i]))
    {
        char c1 = tolower(str1[i]);
        char c2 = tolower(str2[i]);

        if (c1 > c2)
        {
            return 1;
        }

        if (c1 < c2)
        {
            return -1;
        }

        ++i;
    }

    // it means the second string is over but the first still has
    // some characters
    if ('\0' != str1[i])
    {
        return 1;
    }

    if ('\0' != str2[i])
    {
        return -1;
    }

    return 0;
}

int
cl_strncmp(
    IN_READS_Z(length)  char* str1,
    IN_READS_Z(length)  char* str2,
    IN  DWORD length
    )
{
    DWORD i;

    if (NULL == str1)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == str2)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (0 == length)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    i = 0;

    for (i = 0; i < length && '\0' != str1[i] && '\0' != str2[i]; ++i)
    {
        if (str1[i] > str2[i])
        {
            return 1;
        }

        if (str1[i] < str2[i])
        {
            return -1;
        }
    }

    if (i == length)
    {
        // they are truly equal
        return 0;
    }

    if ('\0' != str1[i])
    {
        // string 1 is bigger
        return 1;
    }

    // string 2 is bigger
    return -1;
}

int
cl_strnicmp(
    IN_READS_Z(length)  char* str1,
    IN_READS_Z(length)  char* str2,
    IN  DWORD length
    )
{
    DWORD i;

    if (NULL == str1)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (NULL == str2)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (0 == length)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    i = 0;

    for (i = 0; i < length && '\0' != str1[i] && '\0' != str2[i]; ++i)
    {
        char c1 = tolower(str1[i]);
        char c2 = tolower(str2[i]);

        if (c1 > c2)
        {
            return 1;
        }

        if (c1 < c2)
        {
            return -1;
        }
    }

    if (i == length)
    {
        // they are truly equal
        return 0;
    }

    if ('\0' != str1[i])
    {
        // string 1 is bigger
        return 1;
    }

    // string 2 is bigger
    return -1;
}

const
char*
cl_strchr(
    IN_Z  char* str,
    IN  char c
    )
{
    DWORD i;

    if (NULL == str)
    {
        return NULL;
    }

    i = 0;

    while ('\0' != str[i])
    {
        if (str[i] == c)
        {
            return (str + i);
        }
        ++i;
    }

    return str;
}

const
char*
cl_strrchr(
    IN_Z  char* str,
    IN  char c
    )
{
    DWORD i;
    const char* charIndex;

    if (NULL == str)
    {
        return NULL;
    }

    i = 0;
    charIndex = str;

    while ('\0' != str[i])
    {
        if (str[i] == c)
        {
            charIndex = (str + i);
        }
        ++i;
    }

    return charIndex;
}

void
cl_strcpy(
    OUT_Z char* dst,
    IN_Z  char* src
    )
{
    DWORD i;

    ASSERT( NULL != dst );
    ASSERT( NULL != src );

    i = 0;

    while ('\0' != src[i])
    {
        dst[i] = src[i];
        ++i;
    }

    dst[i] = '\0';
}

void
cl_strncpy(
    OUT_WRITES(length + 1)  char* dst,
    IN_READS_Z(length)      char* src,
    IN                      DWORD length
    )
{
    DWORD i;

    ASSERT( NULL != dst );
    ASSERT( NULL != src );
    ASSERT( 0 != length );

    i = 0;

    for (i = 0; i < length && src[i] != '\0'; ++i)
    {
        dst[i] = src[i];
    }

    dst[i] = '\0';
}

SIZE_SUCCESS
DWORD
cl_strlen(
    IN_Z  char* str
    )
{
    DWORD i;

    if (NULL == str)
    {
        return INVALID_STRING_SIZE;
    }

    i = 0;

    while ('\0' != str[i])
    {
        ++i;
    }

    return i;
}

SIZE_SUCCESS
DWORD
cl_strlen_s(
    IN_READS_Z(maxLen)
                char*   str,
    IN          DWORD   maxLen
    )
{
    DWORD i;

    if (NULL == str)
    {
        return INVALID_STRING_SIZE;
    }

    i = 0;

    while (i < maxLen && '\0' != str[i])
    {
        ++i;
    }

    return i;
}

STATUS
cl_snprintf(
    OUT_WRITES(buffSize)    char* outputBuffer,
    IN                      DWORD buffSize,
    IN_Z                    char* inputBuffer,
    ...
    )
{
    va_list va;

    va_start(va, inputBuffer);

    return cl_vsnprintf(outputBuffer, buffSize, inputBuffer, va);
}

STATUS
cl_vsnprintf(
    OUT_WRITES(buffSize)    char*       outputBuffer,
    IN                      DWORD       buffSize,
    IN_Z                    char*       inputBuffer,
    INOUT                   va_list     argptr
    )
{
    char temp[VSNPRINTF_BUFFER_SIZE];        // temporary buffer
    DWORD index;                // index in Buffer
    DWORD param_index;            // index which we want to find
    QWORD temp_value;
    DWORD outBufferOffset;
    DWORD temp_len;
    char* temp_str;

    if (NULL == outputBuffer)
    {
        return STATUS_INVALID_PARAMETER1;
    }

    if (0 == buffSize)
    {
        return STATUS_INVALID_PARAMETER2;
    }

    if (NULL == inputBuffer)
    {
        return STATUS_INVALID_PARAMETER3;
    }

    index = 0;
    param_index = 1;
    temp_value = 0;
    outBufferOffset = 0;
    temp_len = 0;
    temp_str = temp;

    while ('\0' != inputBuffer[index])
    {
        if ('%' == inputBuffer[index])
        {
            DWORD digits = 0;
            DWORD fillSpaces = 0;
            char next = inputBuffer[index + 1];
            BOOLEAN firstSpecificator = TRUE;
            char fillChar = ' ';
            DWORD charsToCopy;

            while (('0' <= next) && (next <= '9'))
            {
                if (firstSpecificator)
                {
                    firstSpecificator = FALSE;

                    if ('0' == next)
                    {
                        fillChar = '0';
                    }
                }

                digits = digits * 10 + next - '0';
                index++;
                next = inputBuffer[index + 1];
            }


            switch (next)
            {
            case 'b':
                // we have an unsigned 32 bit value to print
                temp_value = va_arg(argptr, DWORD);
                itoa(&temp_value, FALSE, temp_str, BASE_TWO, FALSE);
                break;
            case 'B':
                // we have an unsigned 64 bit value to print
                temp_value = va_arg(argptr, QWORD);
                itoa(&temp_value, FALSE, temp_str, BASE_TWO, TRUE);
                break;
            case 'u':
                // we have an unsigned 32 bit value to print
                temp_value = va_arg(argptr, DWORD);
                itoa(&temp_value, FALSE, temp_str, BASE_TEN, FALSE);
                break;
            case 'U':
                // we have an unsigned 64 bit value to print
                temp_value = va_arg(argptr, QWORD);
                itoa(&temp_value, FALSE, temp_str, BASE_TEN, TRUE);
                break;
            case 'd':
                // we have a signed 32 bit value to print
                temp_value = va_arg(argptr, DWORD);
                itoa(&temp_value, TRUE, temp_str, BASE_TEN, FALSE);
                break;
            case 'D':
                // we have a signed 64 bit value
                temp_value = va_arg(argptr, QWORD);
                itoa(&temp_value, TRUE, temp_str, BASE_TEN, TRUE);
                break;
            case 'x':
                // we have a 32 bit hexadecimal value to print
                temp_value = va_arg(argptr, DWORD);
                itoa(&temp_value, FALSE, temp_str, BASE_HEXA, FALSE);
                break;
            case 'X':
                // we have a 64 bit hexadecimal value to print
                temp_value = va_arg(argptr, QWORD);
                itoa(&temp_value, FALSE, temp_str, BASE_HEXA, TRUE);
                break;
            case 'c':
                // we have a character value to print
                temp_value = va_arg(argptr, char);
                cl_strncpy(temp_str, (char*)&temp_value, sizeof(char));
                break;
            case 's':
            case 'S':
                // we have a string to print
                temp_str = va_arg(argptr, char*);
                break;
            default:
                // A parsing error - an incorrect string was supplied =>
                // return a status error
                return STATUS_PARSE_FAILED;
            }

            temp_len = strlen(temp_str);
            ASSERT(temp_len != INVALID_STRING_SIZE);

            charsToCopy = (next == 'S') ? min(digits, temp_len) : temp_len;

            if (outBufferOffset + temp_len >= buffSize)
            {
                // we don't have any more space
                return STATUS_BUFFER_TOO_SMALL;
            }
            // see if we need to pad the string with spaces or with digits
            fillSpaces = digits > temp_len ? digits - temp_len : 0;
            if (outBufferOffset + temp_len + fillSpaces >= buffSize)
            {
                // we don't have any more space
                return STATUS_BUFFER_TOO_SMALL;
            }
            if (0 != fillSpaces)
            {
                // put spaces
                cl_memset(outputBuffer + outBufferOffset, fillChar, fillSpaces);
                outBufferOffset = outBufferOffset + fillSpaces;
            }

            if (charsToCopy != 0)
            {
                cl_strncpy(outputBuffer + outBufferOffset, temp_str, charsToCopy);
            }
            outBufferOffset = outBufferOffset + charsToCopy;

            param_index++;          // number of parameters parsed
            index += 2;             // we advance in the format string
            temp_str = temp;        // we need to reset temp_str to point to temp
                                    // else, after a string operation it will point to the
                                    // printed string
        }
        else
        {
            if (outBufferOffset + 1 >= buffSize)
            {
                // we don't have any more space
                outputBuffer[outBufferOffset] = '\0';
                return STATUS_BUFFER_TOO_SMALL;
            }

            outputBuffer[outBufferOffset] = inputBuffer[index];

            outBufferOffset++;
            index++;
        }

    }

    outputBuffer[outBufferOffset] = '\0';

    return param_index - 1;
}

const
char*
cl_strtok_s(
    _When_(*context == NULL, IN_Z)
    _When_(*context != NULL, IN_OPT)
                                char*       strToken,
    IN_Z                        char*       delimiters,
    INOUT                       char**      context
    )
{
    DWORD i;
    DWORD j;
    char* pStr;

    if (NULL == delimiters)
    {
        return NULL;
    }

    if (NULL == context)
    {
        return NULL;
    }

    if ((NULL == *context) && (NULL == strToken) )
    {
        return NULL;
    }

    pStr = (NULL == *context) ? strToken : *context;

    if ('\0' == pStr[0])
    {
        // we reached the end of the string
        return NULL;
    }

    for (i = 0; pStr[i] != '\0'; ++i)
    {
        for (j = 0; delimiters[j] != '\0'; ++j)
        {
            if (pStr[i] == delimiters[j])
            {
                pStr[i] = '\0';
                *context = &pStr[i + 1];
                return pStr;
            }
        }
    }

    // we return the whole string
    // if we reached the end of the string the context must point to NULL as well
    // else we go over our string boundary
    *context = pStr[i] == '\0' ? &pStr[i] : &pStr[i + 1];

    return pStr;
}

SIZE_SUCCESS
DWORD
cl_strcelem(
    IN_Z                    char*       string,
    IN                      char        delimiter
    )
{
    DWORD noOfElements;
    const char* pPrevValue;

    if (string == NULL)
    {
        return INVALID_STRING_SIZE;
    }

    // If we have n elements we will have n-1 spaces
    // => start with 1 element
    noOfElements = 1;
    pPrevValue = string;

    for (const char* curPosition = cl_strchr(string, delimiter);
         curPosition != pPrevValue;
         curPosition = cl_strchr(curPosition, delimiter))
    {
        noOfElements++;

        // need to increment by 1 else we'll get stuck in a loop because strchr returns a pointer
        // to the first occurrence of the character
        curPosition++;
        pPrevValue = curPosition;
    }

    return noOfElements;
}

void
cl_strtrim(
    INOUT                    char*       string
    )
{
    DWORD startIndex = MAX_DWORD;

    ASSERT(NULL != string);

    // find the first non-space character
    for (DWORD i = 0; string[i] != '\0'; i++)
    {
        if (!isspace(string[i]))
        {
            startIndex = i;
            break;
        }
    }

    // if only whitespaces were found
    if (MAX_DWORD == startIndex)
    {
        string[0] = '\0';
        return;
    }

    DWORD lengthAfterRightTrim = strlen(string);

    // insert '\0' after last non-space character
    for (DWORD i = strlen(string); i > startIndex; i--)
    {
        if (!isspace(string[i - 1]))
        {
            string[i] = '\0';
            lengthAfterRightTrim = i;
            break;
        }
    }

    // if the first non-space character is not at the beginning of the buffer
    if (0 != startIndex)
    {
        ASSERT(lengthAfterRightTrim != 0);

        // shift all characters to the left
        cl_memmove(string, &string[startIndex], lengthAfterRightTrim);
    }
}
