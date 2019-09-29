#pragma once

C_HEADER_START
#define INVALID_STRING_SIZE             MAX_DWORD

//******************************************************************************
// Function:     strcmp
// Description:  Compares two strings.
// Returns:      int  0 => strings equal
//                   +1 => str1 > str2
//                   -1 => str2 > str1
// Parameter:    IN_Z char * str1
// Parameter:    IN_Z char * str2
//******************************************************************************
int
cl_strcmp(
    IN_Z  char* str1,
    IN_Z  char* str2
    );

//******************************************************************************
// Function:     stricmp
// Description:  Same as strcmp except its case insensitive.
// Returns:      int
// Parameter:    IN_Z char * str1
// Parameter:    IN_Z char * str2
//******************************************************************************
int
cl_stricmp(
    IN_Z  char* str1,
    IN_Z  char* str2
    );

int
cl_strncmp(
    IN_READS_Z(length)  char* str1,
    IN_READS_Z(length)  char* str2,
    IN  DWORD length
    );

int
cl_strnicmp(
    IN_READS_Z(length)  char* str1,
    IN_READS_Z(length)  char* str2,
    IN  DWORD length
    );

//******************************************************************************
// Function:     strchr
// Description:  Searches a string for the first occurrence of a character.
// Returns:      const char* - pointer to the first occurence of the character.
//               If the character is not found the result will point to str.
// Parameter:    IN_Z char * str
// Parameter:    IN char c
//******************************************************************************
const
char*
cl_strchr(
    IN_Z  char* str,
    IN  char c
    );

//******************************************************************************
// Function:     strrchr
// Description:  Searches a string for the last occurrence of a character.
// Returns:      const char* - pointer to the last occurence of the character.
//               If the character is not found the result will point to str.
// Parameter:    IN_Z char * str
// Parameter:    IN char c
//******************************************************************************
const
char*
cl_strrchr(
    IN_Z  char* str,
    IN  char c
    );

void
cl_strcpy(
    OUT_Z char* dst,
    IN_Z  char* src
    );

//******************************************************************************
// Function:     strncpy
// Description:  Copies the first length characters from src to dst. If the src
//               string terminates faster the copying will stop. No matter what
//               the termination reason dst[i+1] will be set to '\0', where i
//               represents the number of characters copied.
// Returns:      void
// Parameter:    char* dst - the size of the dst buffer must be with at laast
//               1 BYTE greater than length
// Parameter:    char* src
// Parameter:    IN DWORD length
//******************************************************************************
void
cl_strncpy(
    OUT_WRITES(length + 1)  char* dst,
    IN_READS_Z(length)      char* src,
    IN                      DWORD length
    );

SIZE_SUCCESS
DWORD
cl_strlen(
    IN_Z  char* str
    );

SIZE_SUCCESS
DWORD
cl_strlen_s(
    IN_READS_Z(maxLen)
                char*   str,
    IN          DWORD   maxLen
    );

// buffSize also includes the NULL terminator
// => only buffSize - 1 will actually be
// characters
STATUS
cl_snprintf(
    OUT_WRITES(buffSize)    char* outputBuffer,
    IN                      DWORD buffSize,
    IN_Z                    char* inputBuffer,
    ...
    );

#define cl_sprintf(outBuff,inBuff,...)     cl_snprintf(outBuff,MAX_PATH,inBuff,__VA_ARGS__)

STATUS
cl_vsnprintf(
    OUT_WRITES(buffSize)    char*       outputBuffer,
    IN                      DWORD       buffSize,
    IN_Z                    char*       inputBuffer,
    INOUT                   va_list     argptr
    );

const
char*
cl_strtok_s(
    _When_(*context==NULL,IN_Z)
    _When_(*context!=NULL,IN_OPT)
                                char*       strToken,
    IN_Z                        char*       delimiters,
    INOUT                       char**      context
    );

SIZE_SUCCESS
DWORD
cl_strcelem(
    IN_Z                    char*       string,
    IN                      char        delimiter
    );

void
cl_strtrim(
    INOUT                    char*       string
    );
C_HEADER_END
