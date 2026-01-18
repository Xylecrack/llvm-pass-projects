#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern uint32_t __cfcss_G;  // Global run-time signature (GSR)

__attribute__((noreturn))
void __cfcss_fault(void) {
    fprintf(stderr, "[CFCSS] Control-flow error detected! Aborting.\n");
    abort();
}
