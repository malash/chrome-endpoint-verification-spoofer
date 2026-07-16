// spoof — set DYLD_INSERT_LIBRARIES + SPOOF_CONFIG, then exec argv[1..].
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <mach-o/dyld.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <program> [args...]\n", argv[0]);
        return 2;
    }

    char exe[PATH_MAX];
    uint32_t sz = sizeof(exe);
    if (_NSGetExecutablePath(exe, &sz) != 0) {
        fprintf(stderr, "spoof: executable path too long\n");
        return 1;
    }
    char resolved[PATH_MAX];
    if (!realpath(exe, resolved)) strlcpy(resolved, exe, sizeof(resolved));
    char *dir = dirname(resolved);

    char dylib[PATH_MAX], conf[PATH_MAX];
    snprintf(dylib, sizeof(dylib), "%s/spoof-lib.dylib", dir);
    snprintf(conf, sizeof(conf), "%s/../spoof.conf", dir);

    setenv("DYLD_INSERT_LIBRARIES", dylib, 1);
    if (!getenv("SPOOF_CONFIG")) setenv("SPOOF_CONFIG", conf, 1);

    execvp(argv[1], &argv[1]);
    perror("spoof: execvp");
    return 127;
}
