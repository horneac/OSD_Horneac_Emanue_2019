#pragma once

C_HEADER_START
// concatenate two BYTEs to make WORD
#define BYTES_TO_WORD(x,y)                  ((((WORD)(x))<<8)| \
                                              ((WORD)(y)))

// concatenate two WORDs to make DWORD
#define WORDS_TO_DWORD(x,y)                 ((((DWORD)(x))<<16)| \
                                              ((DWORD)(y)))

// concatenate two DWORDs to make QWORD
#define DWORDS_TO_QWORD(x,y)                ((((QWORD)(x))<<32)| \
                                              ((QWORD)(y)))


#define WORD_HIGH(x)                        (((WORD)(x))>>8)
#define WORD_LOW(x)                         (((WORD)(x))&MAX_BYTE)

#define DWORD_HIGH(x)                       (((DWORD)(x))>>16)
#define DWORD_LOW(x)                        (((DWORD)(x))&MAX_WORD)

#define QWORD_HIGH(x)                       (((QWORD)(x))>>32)
#define QWORD_LOW(x)                        (((QWORD)(x))&MAX_DWORD)

#ifndef max
#define max(a,b)                            ((a)>(b)?(a):(b))
#endif // max

#ifndef min
#define min(a,b)                            ((a)<(b)?(a):(b))
#endif // min

#define ntohd(x)    ((DWORD)                            \
                    ((((x) >> 24) & 0x0000'00FF)    |   \
                    ( ((x) << 8)  & 0x00FF'0000)    |   \
                    ( ((x) >> 8)  & 0x0000'FF00)    |   \
                    ( ((x) << 24) & 0xFF00'0000)))

#define ntohw(x)    ((WORD)(((x) >> 8) | ((x) << 8)))

#define htond(x)    ntohd((x))
#define htonw(x)    ntohw((x))

QWORD
CalculatePercentage(
    IN      QWORD       WholeValue,
    IN      WORD        HundredsOfPercentage
    );
C_HEADER_END
