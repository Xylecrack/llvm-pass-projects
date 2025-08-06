void test_unsorted(int *A, int n) {
  for (int i = 0; i < n; ++i) {
    A[i + 2] = 0; // Should be last
    A[i] = 1;     // Should be first
    A[i + 1] = 2; // Should be second
  }
}