#include "solver.h"
#include <algorithm>

// typedef struct arguments *arg_t;
struct args {
  int k; 
  int B;
  int *sizes;
  int *values;
};

solver_t S;

key gk(int n, int m, int w) {
  return m * w + n;
}

int recur(void *va) {
  args *a = (args*)va;
  if (a->k == 0) {
    return 0;
  }
  else if (a->sizes[a->k] > a->B) {
    printf("Too big for knapsack\n");
    args b = {a->k - 1, a->B, a->sizes, a->values};
    return solve(S, &b, b.k, b.B);
  }
  else {
    printf("Trying to put it in knapsack\n");
    args b = {a->k - 1, a->B - a->sizes[a->k], a->sizes, a->values};
    int x = solve(S, &b, b.k, b.B);
    
    printf("Trying without putting it in knapsack\n");
    args c = {a->k - 1, a->B, a->sizes, a->values};
    int y = solve(S, &c, c.k, c.B);

    return max(a->values[a->k] + x, y);
  }
}

int main() {
  int sizes[6] = {10, 20, 30};
  int values[6] = {60, 100, 120};
  int w = 3;
  args b = {3, 50, sizes, values};
  S = dp_solver_new(recur, gk, w, b.k, b.B);
  printf("Result: %d\n", solve(S, &b, b.k, b.B));
  delete S;
  return 0;
}