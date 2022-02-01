#include "build.h"
#include <stdio.h>

#ifdef _WIN32
#define CFLAGS                                                                 \
  "/std:c11", "/O2", "/FC", "/W4", "/WX", "/wd4996", "/nologo",                \
      "/Fe.\\build\\bin\\", "/Fo.\\build\\bin\\", "/diagnostics:caret"
#else
#define CFLAGS                                                                 \
  "-Wall", "-Wextra", "-Wswitch-enum", "-Wmissing-prototypes", "-Wconversion", \
      "-Wpedantic", "-fno-strict-aliasing", "-ggdb", "-std=c11"
#endif

const char *toolchain[] = {"basm", "bme", "bmr", "debasm", "bdb", "basm2nasm"};

#ifdef _WIN32
void build_c_file(const char *input_path, const char *output_path) {
  (void)output_path;
  CMD("cl.exe", CFLAGS, input_path);
}
#else
void build_c_file(const char *input_path, const char *output_path) {
  CMD("cc", CFLAGS, "-o", output_path, input_path);
}
#endif

int main() {
  MKDIRS("build", "bin");

  FOREACH_ARRAY(const char *, tool, toolchain, {
    printf("Building %s...\n", CONCAT(tool, ".c"));
    build_c_file(CONCAT(tool, ".c"), PATH("build", "bin", tool));
  });

  MKDIRS("build", "examples");

  FOREACH_FILE_IN_DIR(example, "examples", {
    size_t n = strlen(example);
    assert(n >= 4);
    printf("Building %s...\n", example);
    if (strcmp(example + n - 4, "basm") == 0) {
      CMD(PATH("build", "bin", "basm"), "-g", PATH("examples", example),
          PATH("build", CONCAT(example, ".bm")));
    }
  });
}
