#include "solver.h"
#include <algorithm>

// typedef struct arguments *arg_t;
struct args {
  uint32_t num_vertices;
};

solver_t S;

// cannot have a zero key
key gk(int n, int m, int w) {
  return m * w + n + 1;
}

int recur(void *va) {
  args *a = (args*)va;
  return 0;
}

// TODO: reorganize bellman ford to utilize the recur function
int main() {
  uint32_t num_vertices = 5;
  uint32_t source = 2;
  S = dp_solver_new(recur, gk, num_vertices, num_vertices, num_vertices);
  htable_t H = S->H;
  vector<pair<pair<int, int>, int>> edges (10);

  // Initialize all paths with 0 edges with infinite cost
  for (uint32_t i = 0; i < num_vertices; i++) {
    ht_insert(H, gk(0, i, num_vertices), INT32_MAX);
  }
  
  // Initialize path with 0 edges to source (source -> source) with 0 cost
  ht_insert(H, gk(0, source, num_vertices), 0);

  for (uint32_t k = 1; k < num_vertices; k++) {
    // For all v, d[k][v] = d[k-1][v]
    for (uint32_t v = 0; v < num_vertices; v++) {
      ht_insert(H, gk(k, v, num_vertices), ht_lookup(H, gk(k-1, v, num_vertices)));
    }
    
    for (pair<pair<int, int>, int> edge : edges) {
      int u = edge.first.first;
      int v = edge.first.second;
      int weight = edge.second;
      int x = ht_lookup(H, gk(k, v, num_vertices));
      int y = ht_lookup(H, gk(k-1, u, num_vertices)) + weight;
      ht_insert(H, gk(k, v, num_vertices), min(x, y));
    }
  }

  delete S;
  return 0;

  // https://sy9777m.github.io/Kevin_Min/markdown/2021/11/16/algorithms-week-11.html#bellman-ford
}