#include "build.h"

#include <stdio.h>

int main(void) {
    char* p = PATH("hello", "world");
    printf("%s\n", p);
    free(p);
    return 0;
}
