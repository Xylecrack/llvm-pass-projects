#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t __cfcss_G = 0;  // Global run-time signature (GSR)

__attribute__((noreturn))
void __cfcss_fault(void) {
    fprintf(stderr, "[CFCSS] Control-flow error detected! Aborting.\n");
    abort();
}
