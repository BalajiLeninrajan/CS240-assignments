#include <iostream>
#include <vector>

using namespace std;

struct Node {
  int i = -1, j = -1; // comparison indices
  int idx = -1;       // permutation index

  bool operator==(const Node &other) const {
    return i == other.i && j == other.j && idx == other.idx;
  }

  bool operator!=(const Node &other) const { return !(other == *this); }
};

class DecisionTree;

int factorial(int n);
void insertion_sort(DecisionTree &dtree, vector<int> &A, vector<int> &Alookup,
                    size_t &dt_idx);
void selection_sort(DecisionTree &dtree, vector<int> &A, vector<int> &Alookup,
                    size_t &dt_idx);

class DecisionTree {
private:
  vector<Node> dtree;
  vector<vector<int>> perms;
  vector<vector<int>> inverse_perms;
  string algo_select;
  int algo_n;

  // backtracking helper function to generate all permutations
  void permuations_helper(vector<int> perm, int idx) {
    if (idx == perm.size()) {
      perms.push_back(perm);
      inverse_perms.push_back(make_loopup_vector(perm));
      return;
    }

    for (int i = idx; i < perm.size(); i++) {
      swap(perm[idx], perm[i]);
      permuations_helper(perm, idx + 1);
      swap(perm[idx], perm[i]); // backtrack
    }
  }

  // generates all sorting permutations of n elements
  void generate_permutations(int n) {
    vector<int> perm(n);
    for (int i = 0; i < n; i++)
      perm[i] = i;

    permuations_helper(perm, 0);
  }

  vector<int> make_loopup_vector(const vector<int> &A) {
    vector<int> Alookup(A.size());
    for (size_t i = 0; i < A.size(); i++)
      Alookup[A[i]] = i;
    return Alookup;
  }

public:
  DecisionTree(string s, int n) : algo_select(s), algo_n(n) {
    generate_permutations(n);
    generate_tree();
  }

  void generate_tree() {
    for (int i = 0; i < perms.size(); i++) {
      vector<int> A = perms[i];
      vector<int> Alookup = inverse_perms[i];

      size_t dt_idx = 0;

      if (algo_select == "insertion") {
        insertion_sort(*this, A, Alookup, dt_idx);
      } else if (algo_select == "selection") {
        selection_sort(*this, A, Alookup, dt_idx);
      }

      dtree_add(Node{-1, -1, i}, dt_idx);
    }
  }

  void print_permutations() {
    cout << "Permutations of " << algo_n << " elements:\n";
    for (const auto &perm : perms) {
      for (int val : perm)
        cout << val << " ";

      cout << endl;
    }
  }

  // adds node to the decision tree
  void dtree_add(Node n, size_t idx) {
    if (idx >= dtree.size())
      dtree.resize(idx + 1);

    if (dtree[idx] != n)
      dtree[idx] = n;
  }

  void print_dtree() {
    for (size_t i = 0; i < dtree.size(); i++) {
      const Node &node = dtree[i];

      if (i != 0) {
        const Node &parent = dtree[(i - 1) / 2];
        if (parent.i == -1 && parent.j == -1) {
          continue;
        }
      }

      cout << "node " << i << " :";

      if (node.idx != -1) {
        for (const auto &elem : inverse_perms[node.idx]) {
          cout << " " << elem;
        }
      } else if (node.i != -1 && node.j != -1) {
        cout << " " << "A" << node.j << " < A" << node.i << " ? " << 2 * i + 1
             << " : " << 2 * i + 2;
      } else {
        cout << " not reached";
      }
      cout << endl;
    }
  }

  void debug_print_nodes() {
    cout << endl << "== == == DEBUG == == ==" << endl;
    cout << "Decision Tree for " << algo_select << " with " << algo_n
         << " elements:" << endl;
    for (size_t i = 0; i < dtree.size(); i++) {
      cout << "Node " << i << " | ";
      const Node &node = dtree[i];
      cout << "i: " << node.i << ", j: " << node.j << ", idx: " << node.idx
           << endl;
    }
    cout << "== == == GUBED == == ==" << endl;
  }
};

void insertion_sort(DecisionTree &dtree, vector<int> &A, vector<int> &Alookup,
                    size_t &dt_idx) {
  int n = A.size();

  for (int i = 1; i < n; i++)
    for (int j = i; j > 0; j--) {
      dtree.dtree_add(Node{Alookup[A[j - 1]], Alookup[A[j]], -1}, dt_idx);
      if (A[j - 1] > A[j]) {
        dt_idx = 2 * dt_idx + 1; // left child
        swap(A[j], A[j - 1]);
      } else {
        dt_idx = 2 * dt_idx + 2; // right child
        break;
      }
    }
}

void selection_sort(DecisionTree &dtree, vector<int> &A, vector<int> &Alookup,
                    size_t &dt_idx) {
  int n = A.size();

  for (int i = 0; i < n - 1; i++) {
    int idx = i;
    for (int j = i + 1; j < n; j++) {
      dtree.dtree_add(Node{Alookup[A[idx]], Alookup[A[j]], -1}, dt_idx);
      if (A[j] < A[idx]) {
        dt_idx = 2 * dt_idx + 1; // left child
        idx = j;
      } else {
        dt_idx = 2 * dt_idx + 2;
      }
    }

    swap(A[i], A[idx]);
  }
}

int factorial(int n) {
  if (n <= 1)
    return 1;
  return n * factorial(n - 1);
}

int main(int argc, char *argv[]) {
  // get input from command line
  string algo_select;
  int algo_n;
  cin >> algo_select >> algo_n;
  DecisionTree dt(algo_select, algo_n);
  dt.print_dtree();
  return 0;
}
