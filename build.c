#include "build.h"
#include <stdio.h>

#ifdef _WIN32
#define CFLAGS                                                                 \
  "/std:c11", "/O2", "/FC", "/W4", "/WX", "/analyze", "/wd4996", "/nologo",    \
      "/Fe.\\build\\bin\\", "/Fo.\\build\\bin\\", "/diagnostics:caret"
#else
#define CFLAGS                                                                 \
  "-Wall", "-Wextra", "-Wswitch-enum", "-Wmissing-prototypes", "-Wconversion", \
      "-Wpedantic", "-fno-strict-aliasing", "-ggdb", "-std=c11"
#endif

#ifdef _WIN32
static void build_c_file(const char *input_path, const char *output_path) {
  (void)output_path;
  CMD("cl.exe", CFLAGS, input_path);
}
#else
static void build_c_file(const char *input_path, const char *output_path) {
  CMD("cc", CFLAGS, "-o", output_path, input_path);
}
#endif

int main() {
  MKDIRS("build", "bin");

  printf("Building %s...\n", "example.c");
  build_c_file("example.c", PATH("build", "bin", "example"));
}
