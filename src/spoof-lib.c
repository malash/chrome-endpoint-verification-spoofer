// spoof-lib.c — injected dylib: spoofs serial (IOKit) and macOS version (NSProcessInfo)
// from a config file.

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct { long major, minor, patch; } OSVer; // NSInteger == long (LP64)

static char g_serial[128];
static char g_osver_str[128];
static long g_major, g_minor, g_patch;
static int  g_has_serial, g_has_osver;

static void load_config(void) {
    char path[1024] = {0};
    const char *env = getenv("SPOOF_CONFIG");
    if (env && *env) {
        strlcpy(path, env, sizeof(path));
    } else {
        Dl_info info;
        if (dladdr((void *)load_config, &info) && info.dli_fname) {
            strlcpy(path, info.dli_fname, sizeof(path));
            char *slash = strrchr(path, '/');
            if (slash) *slash = '\0';
            strlcat(path, "/../spoof.conf", sizeof(path));
        }
    }
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *nl = strpbrk(line, "\r\n"); if (nl) *nl = '\0';
        if (line[0] == '#' || line[0] == '\0') continue;
        char *eq = strchr(line, '='); if (!eq) continue;
        *eq = '\0';
        const char *key = line, *val = eq + 1;
        if (strcmp(key, "serial") == 0) {
            strlcpy(g_serial, val, sizeof(g_serial)); g_has_serial = 1;
        } else if (strcmp(key, "os_version") == 0) {
            int mj = 0, mn = 0, pt = 0;
            if (sscanf(val, "%d.%d.%d", &mj, &mn, &pt) >= 2) {
                g_major = mj; g_minor = mn; g_patch = pt; g_has_osver = 1;
            }
        } else if (strcmp(key, "os_version_string") == 0) {
            strlcpy(g_osver_str, val, sizeof(g_osver_str));
        }
    }
    fclose(f);
    if (g_has_osver && g_osver_str[0] == '\0')
        snprintf(g_osver_str, sizeof(g_osver_str), "Version %ld.%ld.%ld", g_major, g_minor, g_patch);
}

static OSVer fake_osv(id self, SEL _cmd) {
    return (OSVer){ g_major, g_minor, g_patch };
}
static id fake_osv_str(id self, SEL _cmd) {
    return ((id (*)(id, SEL, const char *))objc_msgSend)(
        (id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), g_osver_str);
}
static CFTypeRef my_ioregprop(io_registry_entry_t entry, CFStringRef key,
                              CFAllocatorRef allocator, IOOptionBits options) {
    if (g_has_serial && key &&
        CFStringCompare(key, CFSTR("IOPlatformSerialNumber"), 0) == kCFCompareEqualTo)
        return CFStringCreateWithCString(allocator, g_serial, kCFStringEncodingUTF8);
    return IORegistryEntryCreateCFProperty(entry, key, allocator, options);
}
__attribute__((used)) static struct { const void *r; const void *o; }
_interpose_ioregprop __attribute__((section("__DATA,__interpose"))) =
    { (const void *)my_ioregprop, (const void *)IORegistryEntryCreateCFProperty };

static void swizzle(Class c, const char *sel, IMP imp) {
    Method m = class_getInstanceMethod(c, sel_registerName(sel));
    if (m) method_setImplementation(m, imp);
}

__attribute__((constructor))
static void install(void) {
    load_config();
    if (!g_has_osver) return;

    id pi = ((id (*)(id, SEL))objc_msgSend)(
        (id)objc_getClass("NSProcessInfo"), sel_registerName("processInfo"));
    if (!pi) return;
    Class c = object_getClass(pi); // real class may be a subclass, e.g. _NSSwiftProcessInfo
    swizzle(c, "operatingSystemVersion",       (IMP)fake_osv);
    swizzle(c, "operatingSystemVersionString", (IMP)fake_osv_str);
}
