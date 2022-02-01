#include "build.h"
#include <stdio.h>

#define CFLAGS                                                                 \
  "-Wall", "-Wextra", "-Wswitch-enum", "-Wmissing-prototypes", "-Wconversion", \
      "-Wpedantic", "-fno-strict-aliasing", "-ggdb", "-std=c11"

const char *toolchain[] = {"basm", "bme", "bmr", "debasm", "bdb", "basm2nasm"};

int main() {
  MKDIRS("build", "bin");

  FOREACH_ARRAY(const char *, tool, toolchain, {
    printf("Building %s...\n", CONCAT(tool, ".c"));
    CMD("clang", CFLAGS, "-o", PATH("build", "bin", tool),
        PATH("src", CONCAT(tool, ".c")));
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
