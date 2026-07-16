// check.c — prints the serial and macOS version this process sees.

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <stdio.h>

extern void *objc_autoreleasePoolPush(void);
extern void  objc_autoreleasePoolPop(void *pool);

int main(void) {
    void *pool = objc_autoreleasePoolPush();

    id pi = ((id (*)(id, SEL))objc_msgSend)(
        (id)objc_getClass("NSProcessInfo"), sel_registerName("processInfo"));
    id ver = ((id (*)(id, SEL))objc_msgSend)(pi, sel_registerName("operatingSystemVersionString"));
    const char *vs = ((const char *(*)(id, SEL))objc_msgSend)(ver, sel_registerName("UTF8String"));
    printf("os_version = %s\n", vs ? vs : "?");

    io_service_t svc = IOServiceGetMatchingService(kIOMainPortDefault,
                                                   IOServiceMatching("IOPlatformExpertDevice"));
    CFStringRef s = IORegistryEntryCreateCFProperty(svc, CFSTR("IOPlatformSerialNumber"),
                                                    kCFAllocatorDefault, 0);
    char buf[128] = {0};
    if (s) CFStringGetCString(s, buf, sizeof(buf), kCFStringEncodingUTF8);
    printf("serial     = %s\n", buf);
    if (s) CFRelease(s);
    if (svc) IOObjectRelease(svc);

    objc_autoreleasePoolPop(pool);
    return 0;
}
