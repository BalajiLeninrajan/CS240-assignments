#include <climits>
#include <cmath>
#include <iostream>
#include <queue>
#include <vector>

using namespace std;

int MinDiff(vector<int> arr) {
  int n = arr.size();
  vector<queue<int>> A(n);
  for (int i = 0; i < n; ++i) {
    A[i].push(arr[i]);
  }

  for (int k = 0; k <= 42; ++k) {
    vector<queue<int>> B(n);
    for (int i = 0; i < n; ++i) {
      while (!A[i].empty()) {
        B[(A[i].front() / (int)pow(n, k)) % n].push(A[i].front());
        A[i].pop();
      }
    }
    A = B;
  }

  vector<int> A_sorted(n);
  int idx = 0;
  for (int i = 0; i < n; ++i) {
    while (!A[i].empty()) {
      A_sorted[idx++] = A[i].front();
      A[i].pop();
    }
  }

  int min_diff = INT_MAX;
  for (int i = 0; i < n - 1; ++i) {
    min_diff = min(min_diff, A_sorted[i + 1] - A_sorted[i]);
  }
  return min_diff;
}

int main() {
  int n;
  cin >> n;
  vector<int> arr(n);
  for (int i = 0; i < n; ++i) {
    cin >> arr[i];
  }
  cout << MinDiff(arr) << endl;
  return 0;
}
