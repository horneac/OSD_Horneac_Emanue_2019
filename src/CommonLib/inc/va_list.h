#pragma once

C_HEADER_START
#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
typedef PBYTE               va_list;
#endif

#define STACKITEM_SIZE      sizeof(PVOID)

// Initializes the va_list
#define va_start(List,LastArg)     \
            ((List)=((va_list)&(LastArg) + STACKITEM_SIZE))

// Retrieves the value of the next argument
// And increases the List pointer
#define va_arg(List, Type)	\
	((List) += STACKITEM_SIZE, *((Type *)((List) - STACKITEM_SIZE)))
C_HEADER_END
