void loop_unsorted(int *arr, int n) {
  for (int i = 0; i < 3; ++i) {
    arr[2 * i + 1] = i + 1; // Access at 1, 3, 5
    arr[2 * i] = i;         // Access at 0, 2, 4
  }
}
