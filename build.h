#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif
#define PATH_SEPARATOR_LENGTH (sizeof PATH_SEPARATOR - 1)

char* path_impl(int ignore, ...) {
    size_t length = 0;
    size_t seps = 0;

    va_list args;
    va_start(args, ignore);
    const char* arg = va_arg(args, const char*);
    while (arg != NULL) {
        length += strlen(arg);
        arg = va_arg(args, const char*);
        if (arg != NULL) {
            seps += 1;
        }
    }
    va_end(args);

    if (length == 0) {
        return "";
    }

    char* result = malloc(length + seps * PATH_SEPARATOR_LENGTH + 1);
    char* p = result;

    va_start(args, ignore);
    arg = va_arg(args, const char*);
    while (arg != NULL) {
        size_t len = strlen(arg);
        memcpy(p, arg, len);
        p += len;
        arg = va_arg(args, const char*);
        if (arg != NULL) {
            memcpy(p, PATH_SEPARATOR, PATH_SEPARATOR_LENGTH);
            p += PATH_SEPARATOR_LENGTH;
        }
    }
    *p = '\0';
    va_end(args);

    return result;
}

#define PATH(...) path_impl(69, __VA_ARGS__, NULL)
