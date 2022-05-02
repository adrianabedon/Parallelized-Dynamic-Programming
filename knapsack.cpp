#include "htable/htable.h"
#include <vector>
#include <algorithm>
#include <utility>
#include <cassert> 
#include <random>
#include <iostream>
#include <cmath>
using namespace std;

typedef struct {
  int n;
  int w;
} subp_t;

typedef struct {
  subp_t problem;
  int *write_result;
  bool waiting_for_results;
  vector<int> results;
} job_t;

int get_key(subp_t problem, int max_weight) {
  return problem.n*(max_weight+1)+problem.w;
}

int knapsack(htable_t H, int max_weight, vector<int> &weights, vector<int> &values, int size_of_sack) {
  random_device dev;
  mt19937 rng(dev());
  uniform_int_distribution<mt19937::result_type> rand_indx(0,1);

  int output_result;
  vector<job_t> jobs;
  jobs.push_back(job_t());
  jobs.back().problem.n = size_of_sack;
  jobs.back().problem.w = max_weight;
  jobs.back().write_result = &output_result;
  jobs.back().waiting_for_results = false;

  while(true) 
  {
    size_t curr_idx = jobs.size() - 1;
    int key = get_key(jobs[curr_idx].problem, max_weight);
    int best;
    /* Check if waiting for results (was already called and subproblems were
     * pushed to the stack)*/
    if(jobs[curr_idx].waiting_for_results) 
    {
      best = max(values[jobs[curr_idx].problem.n-1]+jobs[curr_idx].results[0], jobs[curr_idx].results[1]);
      ht_insert(H, key, best);
      assert(best != EMPTY);
    } 
    else if(jobs[curr_idx].problem.n == 0 || jobs[curr_idx].problem.w == 0) 
    {
      best = 0;
    }
    else 
    {
      best = ht_lookup(H, key);
    }

    if(best != EMPTY) {
      jobs.pop_back();
      *jobs[curr_idx].write_result = best;
      if(jobs[curr_idx].write_result == &output_result) {
        break;
      }
      continue;
    }

    /* Add new job */

    jobs[curr_idx].waiting_for_results = true;
    jobs[curr_idx].results.resize(2);
    
    
    // for other types of problems do with loop
    int random = (int)rand_indx(rng);
    if(random == 0) {
      if(weights[jobs[curr_idx].problem.n-1] <= jobs[curr_idx].problem.w) {
        jobs.push_back(job_t());
        jobs.back().problem.n = jobs[curr_idx].problem.n-1;
        jobs.back().problem.w = jobs[curr_idx].problem.w-weights[jobs[curr_idx].problem.n-1];
        jobs.back().write_result = &jobs[curr_idx].results[0];
        jobs.back().waiting_for_results = false;
      } else {
        jobs[curr_idx].results[0] = INT_MIN;
      }
      
      
      jobs.push_back(job_t());
      jobs.back().problem.n = jobs[curr_idx].problem.n-1;
      jobs.back().problem.w = jobs[curr_idx].problem.w;
      jobs.back().write_result = &jobs[curr_idx].results[1];
      jobs.back().waiting_for_results = false;
    } else {
      jobs.push_back(job_t());
      jobs.back().problem.n = jobs[curr_idx].problem.n-1;
      jobs.back().problem.w = jobs[curr_idx].problem.w;
      jobs.back().write_result = &jobs[curr_idx].results[1];
      jobs.back().waiting_for_results = false;
      
      if(weights[jobs[curr_idx].problem.n-1] <= jobs[curr_idx].problem.w) {
        jobs.push_back(job_t());
        jobs.back().problem.n = jobs[curr_idx].problem.n-1;
        jobs.back().problem.w = jobs[curr_idx].problem.w-weights[jobs[curr_idx].problem.n-1];
        jobs.back().write_result = &jobs[curr_idx].results[0];
        jobs.back().waiting_for_results = false;
      } else {
        jobs[curr_idx].results[0] = INT_MIN;
      }
    }
  }
  return output_result;
}

inline static uint32_t murmur_hash(key_t h) {
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

int main() {
  htable_t H = ht_new(67108864, murmur_hash);
  // output 220
  // vector<int> weights{10, 20, 30};
  // vector<int> values{60, 100, 120};
  // int cap = 50;

  // output 295
  // vector<int> weights{95, 4, 60, 32, 23, 72, 80, 62, 65, 46};
  // vector<int> values{55, 10, 47, 5, 4, 50, 8, 61, 85, 87};
  // int cap = 269;

  // output 1024
  // vector<int> weights{92, 4, 43, 83, 84, 68, 92, 82, 6, 44, 32, 18, 56, 83, 25, 96, 70, 48, 14, 58};
  // vector<int> values{44, 46, 90, 72, 91, 40, 75, 35, 8, 54, 78, 40, 77, 15, 61, 17, 75, 29, 75, 63};
  // int cap = 878;

  // 15,170
  vector<int> weights{54, 95, 36, 18, 4, 71, 83, 16, 27, 84, 88, 45, 94, 64, 14, 80, 4, 23,
    75, 36, 90, 20, 77, 32, 58, 6, 14, 86, 84, 59, 71, 21, 30, 22, 96, 49, 81,
    48, 37, 28, 6, 84, 19, 55, 88, 38, 51, 52, 79, 55, 70, 53, 64, 99, 61, 86,
    1, 64, 32, 60, 42, 45, 34, 22, 49, 37, 33, 1, 78, 43, 85, 24, 96, 32, 99,
    57, 23, 8, 10, 74, 59, 89, 95, 40, 46, 65, 6, 89, 84, 83, 6, 19, 45, 59,
    26, 13, 8, 26, 5, 9};
  vector<int> values{297, 295, 293, 292, 291, 289, 284, 284, 283, 283, 281, 280, 279,
    277, 276, 275, 273,264, 260, 257, 250, 236, 236, 235, 235, 233, 232,
    232, 228, 218, 217, 214, 211, 208, 205, 204, 203, 201, 196, 194, 193,
    193, 192, 191, 190, 187, 187, 184, 184, 184, 181, 179, 176, 173, 172,
    171, 160, 128, 123, 114, 113, 107, 105, 101, 100, 100, 99, 98, 97, 94,
    94, 93, 91, 80, 74, 73, 72, 63, 63, 62, 61, 60, 56, 53, 52, 50, 48, 46,
    40, 40, 35, 28, 22, 22, 18, 15, 12, 11, 6, 5};
  int cap = 3818;

  assert(weights.size() == values.size());
  printf("result: %d\n", knapsack(H, cap, weights, values, (int)weights.size()));
  ht_free(H);
  return 0;
}
