#pragma once

#define LOG(buf,...)                printf("[%s][%u]"##buf, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(buf, ...)         LOG("[ERROR]"##buf, __VA_ARGS__)
#define LOG_FUNC_ERROR(func,err)    LOG_ERROR("Function [%s] failed with status 0x%X\n", (func), (err))
