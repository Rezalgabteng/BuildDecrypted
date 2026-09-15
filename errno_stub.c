/*
 * errno_stub.c - Stub for Android __errno symbol
 * Allows linking against Android-compiled libparadise_api.a on Ubuntu
 */

#include <errno.h>

/* Android uses thread-local __errno, but glibc has errno as macro.
   This provides a dummy implementation so the linker doesn't fail. */

int __errno = 0;

/* Alternative: provide weak stub for errno access */
__attribute__((weak)) int* __errno_location(void) {
    return &__errno;
}
