#include "build.h"

#include <stdio.h>

int main(void) {
    mkdirs(PATH("hello", "world"));
    return 0;
}
