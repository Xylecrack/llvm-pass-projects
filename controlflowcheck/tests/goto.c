#include <stdio.h>

int main() {
    int x = 5;
    if (x > 3) {
        goto label;
    }
    printf("Not jumped\n");
    return 0;

label:
    printf("Jumped here\n");
    return 0;
}
