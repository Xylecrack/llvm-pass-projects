void foo(int a[], int n) {
  int i;
  for (i = 0; i < n;i= i + 3) {
    a[i] = i;
    a[i + 1] = i + 1;
    a[i + 2] = i + 2;
  }
}