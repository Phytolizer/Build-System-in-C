#include "build.hpp"

int main() {
    mkdirs(path("build", "bin"));
    cmd("gcc", "--version");
}
