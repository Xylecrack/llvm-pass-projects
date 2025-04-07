void foo4(int *arr, int n) {
    int i = 0;
    if (n <= 0) return;
    do {
        (arr[i] = i) ? 0 : 0;
        ++i;
    } while (i < n);
}
