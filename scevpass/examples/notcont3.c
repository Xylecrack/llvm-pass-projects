void bar2(int *arr, int n) {
    for (int i = n - 1; i >= 0; i--)
        arr[i * 2] = i;
}
