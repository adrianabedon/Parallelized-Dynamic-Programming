#include "htable.h"
#include <vector>
#include <algorithm>
#include <utility>
#include <cassert> 
#include <random>
#include <iostream>
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

int knapsack(htable_t H, int max_weight, vector<int> weights, vector<int> values, int size_of_sack) {
  random_device dev;
  mt19937 rng(dev());
  uniform_int_distribution<mt19937::result_type> rand_indx(0,1);

  int output_result;
  vector<job_t> jobs;
  jobs.push_back(job_t());
  job_t *curr_job = &jobs.back();
  curr_job->problem.n = size_of_sack;
  curr_job->problem.w = max_weight;
  curr_job->write_result = &output_result;
  curr_job->waiting_for_results = false;


  while(true) 
  {
    curr_job = &jobs.back();
    int key = get_key(curr_job->problem, max_weight);
    int best;
    /* Check if waiting for results (was already called and subproblems were
     * pushed to the stack)*/
    if(curr_job->waiting_for_results) 
    {
      best = max(values[curr_job->problem.n-1]+curr_job->results[0], curr_job->results[1]);
      ht_insert(H, key, best);
      assert(best != EMPTY);
    } 
    else if(curr_job->problem.n == 0 || curr_job->problem.w == 0) 
    {
      best = 0;
    }
    else 
    {
      best = ht_lookup(H, key);
    }

    if(best != EMPTY) {
      jobs.pop_back();
      *curr_job->write_result = best;
      if(curr_job->write_result == &output_result) {
        break;
      }
      continue;
    }

    /* Add new job */

    curr_job->waiting_for_results = true;
    curr_job->results.resize(2);
    
    
    // for other types of problems do with loop
    int random = rand_indx(rng);
    if(random == 0) {
      jobs.push_back(job_t());
      job_t *new_job = &jobs.back();
      new_job->problem.n = curr_job->problem.n-1;
      new_job->problem.w = curr_job->problem.w-weights[curr_job->problem.n];
      new_job->write_result = &curr_job->results[0];
      new_job->waiting_for_results = false;
      
      jobs.push_back(job_t());
      new_job = &jobs.back();
      new_job->problem.n = curr_job->problem.n-1;
      new_job->problem.w = curr_job->problem.w;
      new_job->write_result = &curr_job->results[1];
      new_job->waiting_for_results = false;
    } else {
      jobs.push_back(job_t());
      job_t *new_job = &jobs.back();
      new_job->problem.n = curr_job->problem.n-1;
      new_job->problem.w = curr_job->problem.w;
      new_job->write_result = &curr_job->results[1];
      new_job->waiting_for_results = false;
      
      jobs.push_back(job_t());
      new_job = &jobs.back();
      new_job->problem.n = curr_job->problem.n-1;
      new_job->problem.w = curr_job->problem.w-weights[curr_job->problem.n];
      new_job->write_result = &curr_job->results[0];
      new_job->waiting_for_results = false;
    }
  }
}