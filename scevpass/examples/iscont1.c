void foo1(int *arr, int n) {
    int *p = arr;
    for (int i = 0; i < n; ++i)
        *(p++) = i;
}
