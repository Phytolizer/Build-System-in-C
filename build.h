#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#define FOREACH_VARGS(param, arg, args, body)                                                      \
    do {                                                                                           \
        va_start(args, param);                                                                     \
        for (const char* arg = va_arg(args, const char*); arg != NULL;                             \
                arg = va_arg(args, const char*)) {                                                 \
            body;                                                                                  \
        }                                                                                          \
    } while (0)

#define FOREACH_ARRAY(type, item, items, body)                                                     \
    do {                                                                                           \
        for (size_t i = 0; i < sizeof items / sizeof(type); i += 1) {                              \
            type item = items[i];                                                                  \
            body;                                                                                  \
        }                                                                                          \
    } while (0)

#ifdef _WIN32
#define FOREACH_FILE_IN_DIR(file, dir, body)                                                       \
    do {                                                                                           \
        const char* file = NULL;                                                                   \
        WIN32_FIND_DATA data;                                                                      \
        HANDLE h = FindFirstFile(PATH(dir, "*"), &data);                                           \
        if (h != INVALID_HANDLE_VALUE) {                                                           \
            do {                                                                                   \
                if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {                         \
                    file = data.cFileName;                                                         \
                    body;                                                                          \
                }                                                                                  \
            } while (FindNextFile(h, &data));                                                      \
            FindClose(h);                                                                          \
        }                                                                                          \
    } while (0)
#else
#define FOREACH_FILE_IN_DIR(file, dir, body)                                                       \
    do {                                                                                           \
        struct dirent* entry;                                                                      \
        const char* file = NULL;                                                                   \
        DIR* dirp = opendir(dir);                                                                  \
        if (dirp == NULL) {                                                                        \
            fprintf(stderr, "Failed to open directory %s: %s\n", dir, strerror(errno));            \
            exit(EXIT_FAILURE);                                                                    \
        }                                                                                          \
        while ((entry = readdir(dirp)) != NULL) {                                                  \
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)               \
                continue;                                                                          \
            file = entry->d_name;                                                                  \
            body;                                                                                  \
        }                                                                                          \
        closedir(dirp);                                                                            \
    } while (0)
#endif

const char* concat_sep_impl(const char* sep, ...) {
    const size_t sep_len = strlen(sep);
    size_t length = 0;
    size_t seps_count = 0;

    va_list args;
    FOREACH_VARGS(sep, arg, args, {
        length += strlen(arg);
        seps_count += 1;
    });
    seps_count -= 1;

    assert(length > 0);
    char* result = calloc(length + seps_count * sep_len + 1, 1);
    if (result == NULL) {
        return NULL;
    }

    length = 0;
    FOREACH_VARGS(sep, arg, args, {
        size_t n = strlen(arg);
        memcpy(result + length, arg, n);
        length += n;

        if (seps_count > 0) {
            memcpy(result + length, sep, sep_len);
            length += sep_len;
            seps_count -= 1;
        }
    });

    return result;
}

#define CONCAT_SEP(...) concat_sep_impl(__VA_ARGS__, NULL)

#define PATH(...) CONCAT_SEP("/", __VA_ARGS__, NULL)

_Bool mkdir_cross(const char* path, const int mode) {
#ifdef _WIN32
    (void)mode;
    return CreateDirectory(path, NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(path, mode) >= 0 || errno == EEXIST;
#endif
}

void mkdirs_impl(int ignore, ...) {
    size_t length = 0;
    size_t seps_count = 0;

    va_list args;
    FOREACH_VARGS(ignore, arg, args, {
        length += strlen(arg);
        seps_count += 1;
    });
    seps_count -= 1;

    assert(length > 0);
    char* result = calloc(length + seps_count * 1 + 1, 1);
    if (result == NULL) {
        return;
    }
    length = 0;

    FOREACH_VARGS(ignore, arg, args, {
        size_t n = strlen(arg);
        memcpy(result + length, arg, n);
        length += n;

        if (seps_count > 0) {
            memcpy(result + length, "/", 1);
            length += 1;
            seps_count -= 1;
        }

        if (!mkdir_cross(result, 0755)) {
            fprintf(stderr, "[ERROR] could not create directory %s\n", result);
            exit(1);
        }
    });
}

#define MKDIRS(...) mkdirs_impl(69, __VA_ARGS__, NULL)

const char* concat_impl(int ignore, ...) {
    size_t length = 0;

    va_list args;
    FOREACH_VARGS(ignore, arg, args, { length += strlen(arg); });

    assert(length > 0);
    char* result = calloc(length + 1, 1);
    if (result == NULL) {
        return NULL;
    }

    length = 0;
    FOREACH_VARGS(ignore, arg, args, {
        size_t n = strlen(arg);
        memcpy(result + length, arg, n);
        length += n;
    });

    return result;
}

#define CONCAT(...) concat_impl(69, __VA_ARGS__, NULL)

void cmd_impl(int ignore, ...) {
#ifdef _WIN32
    size_t length = 0;
    size_t seps = 0;
    va_list args;
    FOREACH_VARGS(ignore, arg, args, {
        length += strlen(arg);
        seps += 1;
    });
    char* command_line = calloc(length + seps + 3, 1);
    if (command_line == NULL) {
        fprintf(stderr, "[ERROR] could not allocate memory for command line\n");
        exit(1);
    }
    size_t cursor = 0;
    bool first = true;
    FOREACH_VARGS(ignore, arg, args, {
        if (first) {
            command_line[cursor] = '"';
            cursor += 1;
        }
        size_t n = strlen(arg);
        memcpy(command_line + cursor, arg, n);
        cursor += n;
        if (first) {
            command_line[cursor] = '"';
            cursor += 1;
        }
        command_line[cursor] = ' ';
        cursor += 1;
        first = false;
    });

    STARTUPINFO startup_info = {
            .cb = sizeof(STARTUPINFO),
            .lpReserved = NULL,
            .lpDesktop = NULL,
            .lpTitle = NULL,
            .dwFlags = 0,
            .cbReserved2 = 0,
            .lpReserved2 = NULL,
    };
    PROCESS_INFORMATION process_info = {0};
    BOOL create_process_result = CreateProcess(
            NULL, command_line, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info);
    if (create_process_result == 0) {
        fprintf(stderr, "[ERROR] could not execute command %s\n", command_line);
        fprintf(stderr, "[ERROR] code %lu\n", GetLastError());
        exit(1);
    }
    WaitForSingleObject(process_info.hProcess, INFINITE);
    DWORD exit_code = 0;
    if (GetExitCodeProcess(process_info.hProcess, &exit_code) == 0) {
        fprintf(stderr, "[ERROR] could not get exit code for command %s\n", command_line);
        fprintf(stderr, "[ERROR] code %lu\n", GetLastError());
        exit(1);
    }
    if (exit_code != 0) {
        fprintf(stderr, "[WARN] command %s failed with exit code %lu\n", command_line, exit_code);
    }
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
#else
    size_t argc = 0;
    va_list args;
    FOREACH_VARGS(ignore, arg, args, { argc += 1; });

    char** argv = calloc(argc + 1, sizeof(char*));

    argc = 0;
    FOREACH_VARGS(ignore, arg, args, {
        size_t n = strlen(arg);
        argv[argc] = calloc(n + 1, 1);
        memcpy(argv[argc], arg, n);
        argc += 1;
    });

    pid_t cpid = fork();
    if (cpid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "[ERROR] could not execute command %s\n", argv[0]);
        perror("execvp");
    } else if (cpid < 0) {
        fprintf(stderr, "[ERROR] could not fork a child process\n");
        perror("fork");
        exit(1);
    }

    wait(NULL);
#endif
}

#define CMD(...)                                                                                   \
    do {                                                                                           \
        printf("[CMD] %s\n", CONCAT_SEP(" ", __VA_ARGS__));                                        \
        cmd_impl(69, __VA_ARGS__, NULL);                                                           \
    } while (0)
