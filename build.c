#include "build.h"

#include <stdio.h>

int main(void) {
    mkdirs(PATH("build"));
    return 0;
}
