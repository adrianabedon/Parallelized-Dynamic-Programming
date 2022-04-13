#include "htable.h"

#ifndef DP_SOLVER_H
#define DP_SOLVER_H

typedef struct dp_solver *solver_t;

typedef void *params;
typedef int (*recurrence) (params);

// TODO: change types to int* to allow for n-dimensional dp problems
typedef key (*get_key) (int, int, int);

struct dp_solver {
  recurrence rf;
  get_key gk;
  htable_t H;
  uint32_t width;
};


// TODO: change type of args to int* to allow for n-dimensional dp problems
solver_t dp_solver_new(recurrence rf, get_key gk, uint32_t width, int n, int m);

int solve(solver_t S, params p, int n, int m);

#endif