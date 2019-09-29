#pragma once

#include "cl_string.h"

#ifndef CL_NON_NATIVE

#define strcmp                  cl_strcmp
#define stricmp                 cl_stricmp
#define strncmp                 cl_strncmp
#define strchr                  cl_strchr
#define strrchr                 cl_strrchr
#define strcpy                  cl_strcpy
#define strncpy                 cl_strncpy
#define strlen                  cl_strlen
#define strlen_s                cl_strlen_s
#define snprintf                cl_snprintf
#define sprintf                 cl_sprintf
#define vsnprintf               cl_vsnprintf
#define strtok_s                cl_strtok_s
#define strcelem                cl_strcelem
#define strtrim                 cl_strtrim

#endif // CL_NON_NATIVE
