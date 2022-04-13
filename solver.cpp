#include "solver.h"

/* PRIVATE HELPER FUNCTIONS */

inline static uint32_t murmur_hash(key h) {
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

/* INTERFACE FUNCTIONS */

int solve(solver_t S, params p, int n, int m) {
  key k = S->gk(n, m, S->width);
  int result = ht_lookup(S->H, k);
  if (result != EMPTY) {
    printf("Found index [%d][%d] = %d in table\n", n, m, result);
    return result;
  }
  printf("Solving index [%d][%d]...\n", n, m);
  result = S->rf(p);
  printf("Solved index [%d][%d] = %d\n", n, m, result);
  ht_insert(S->H, k, result);
  return result;
}

solver_t dp_solver_new(recurrence rf, get_key gk, uint32_t width, int n, int m) {
  solver_t S = new struct dp_solver;
  uint32_t cap = n * m;
  S->H = ht_new(cap, murmur_hash);
  S->rf = rf;
  S->gk = gk;
  S->width = width;
  return S;
}