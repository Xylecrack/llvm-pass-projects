void foo2(int *arr, int n) {
    for (int i = 0; i < n; i += 2) {
        arr[i] = i;
        if (i + 1 < n) arr[i + 1] = i + 1;
    }
}
