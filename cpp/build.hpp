#pragma once

#include <algorithm>
#include <concepts>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#ifdef _WIN32
constexpr std::string_view path_sep = "\\";
#else
constexpr std::string_view path_sep = "/";
#endif

template <std::convertible_to<std::string_view>... Segments>
std::string path(Segments&&... segments) {
    std::vector<std::string_view> segments_vec;
    (segments_vec.push_back(segments), ...);
    std::ostringstream result;
    for (std::size_t i = 0; auto segment : segments_vec) {
        result << segment;
        if (i < segments_vec.size() - 1) {
            result << path_sep;
        }
        i += 1;
    }
    return result.str();
}

static inline void mkdir_or_die(const std::string& path) {
    std::cout << "[INFO] mkdir: " << path << '\n';
    int result = mkdir(path.c_str(), 0755);
    if (result < 0 && errno != EEXIST) {
        std::perror("mkdir");
        std::exit(EXIT_FAILURE);
    }
}

static inline void mkdirs(std::string path) {
    auto slash = path.begin();
    while (true) {
        slash = std::find(slash, path.end(), '/');
        if (slash == path.end()) {
            mkdir_or_die(path);
            break;
        }

        mkdir_or_die({path.begin(), slash});
        slash += path_sep.size();
    }
}

static inline char* string_dup(std::string_view arg) {
    char* result = new char[arg.size() + 1];
    std::memcpy(result, arg.data(), arg.size());
    result[arg.size()] = '\0';
    return result;
}

template <std::convertible_to<std::string_view>... Args>
void cmd(const std::string& command, Args&&... args) {
    std::vector<char*> args_vec;
    args_vec.push_back(string_dup(command));
    (args_vec.push_back(string_dup(args)), ...);
    args_vec.push_back(nullptr);

    std::cout << "[CMD]";
    for (char* arg : args_vec) {
        if (arg != nullptr) {
            std::cout << ' ' << arg;
        }
    }
    std::cout << '\n';

    pid_t pid = fork();
    if (pid < 0) {
        std::perror("fork");
        std::exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        execvp(command.c_str(), args_vec.data());
        std::perror("execvp");
        std::exit(EXIT_FAILURE);
    }

    for (char* arg : args_vec) {
        delete[] arg;
    }

    int status;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        std::cerr << "[ERROR] Subcommand failed\n";
        std::exit(EXIT_FAILURE);
    }
}
