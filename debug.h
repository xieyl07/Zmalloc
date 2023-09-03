#pragma once

#include <cstdio>
#include <cstdarg>
#include <thread>
#include <iostream>

static void debug_print(const char *file_name, const char* func_name,
                        int line_num, const char *format, ...)
    {
    fprintf(stderr, "%s[%d]:%s()", file_name, line_num, func_name);
    std::cerr << " <tid " << std::this_thread::get_id();
    std::cerr << ">{";
    va_list va;
    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);
    fprintf(stderr, "}\n");
}

#ifndef release_no_debug
#   define deO(format, ...) /* debug output */ \
        debug_print(__FILE_NAME__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);
#else
#   define deO(format, ...) /* debug output */ ;
#endif