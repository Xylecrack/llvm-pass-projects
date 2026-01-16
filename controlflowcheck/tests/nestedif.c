#include <stdio.h>

int main() {
    int x = 10;
    if (x > 5) {
        printf("Greater than 5\n");
        if (x > 8) {
            printf("Also greater than 8\n");
        }
    }
    return 0;
}
