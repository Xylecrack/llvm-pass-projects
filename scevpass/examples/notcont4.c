#include <stdio.h>

void tricky_disc(int *arr, int len) {
    int i = 0, k = 0;
    while ((i ^ len) != 0) {
        *(arr + ((k * 3) - (k % 2))) = i;
        i = (i & 0xfffffff) + 1;
        k = (k + 2) & 0xfff;
    }
}
