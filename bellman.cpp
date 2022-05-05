#include "htable/htable.h"
#include <vector>
#include <algorithm>
#include <utility>
#include <cassert> 
#include <random>
#include <iostream>
#include <cmath>
#include <chrono>
#include <unistd.h>
#include <math.h>

// #define TIME_CODE

using namespace std;
using namespace std::chrono;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::duration<double> dsec;

typedef struct {
  int v;
  int k;
} subp_t;

typedef struct {
  int u, v, weight;
} edge_t;

typedef struct {
  int V, E;
  vector< vector<edge_t> > parents; // entry v will have all edges (0,v),(3,v),etc
                                    // edges from parent->v
} graph_t;

typedef struct {
  subp_t problem;
  int *write_result;
  bool waiting_for_results;
  vector<int> results;
} job_t;

typedef struct {
  pthread_cond_t *cond;
  pthread_mutex_t *mutex;
  int *done_thread;
  int thread_id;
  int *write_result;
  htable_t H;
  graph_t *G;
  int source;
  int dest;

  pthread_cond_t *cond_start;
  pthread_mutex_t *mutex_start;
  bool *start;
} thread_arg_t;

int get_key(subp_t problem, int V) {
  return problem.k+problem.v*V;
}

int bellman(htable_t H, graph_t *G, int source, int dest) {
  random_device dev;
  mt19937 rng(dev());
  uniform_int_distribution<mt19937::result_type> rand_indx(0,1);

  int output_result;
  vector<job_t> jobs;
  jobs.push_back(job_t());
  jobs.back().problem.v = dest;
  jobs.back().problem.k = G->V-1;
  jobs.back().write_result = &output_result;
  jobs.back().waiting_for_results = false;

  #ifdef TIME_CODE
  double total_lookup_time = 0;
  double total_job_time = 0;
  double total_insertion_time = 0;
  #endif

  while(true) 
  {
    size_t curr_idx = jobs.size() - 1;
    int curr_v = jobs[curr_idx].problem.v;
    int curr_k = jobs[curr_idx].problem.k;
    int key = get_key(jobs[curr_idx].problem, G->V);
    int best;
    /* Check if waiting for results (was already called and subproblems were
     * pushed to the stack)*/
    if(jobs[curr_idx].waiting_for_results) 
    {
      assert(jobs[curr_idx].results.size() > 0);
      best = INT_MAX;
      for(size_t i = 0 ; i < jobs[curr_idx].results.size() ; i++) {
        best = min(jobs[curr_idx].results[i], best);
      }
      #ifdef TIME_CODE
      auto start = Clock::now();
      ht_insert(H, key, best);
      auto end = Clock::now();
      total_insertion_time += duration_cast<dsec>(end - start).count();
      #else
      ht_insert(H, key, best);
      #endif

      assert(best != INT_MIN);
    }
    else if(curr_k == 0 && curr_v == source)
    {
      best = 0;
    }
    else if(curr_k == 0 && curr_v != source) 
    {
      best = INT_MAX;
    }
    else
    {
      #ifdef TIME_CODE
      auto start = Clock::now();
      best = ht_lookup(H, key);
      auto end = Clock::now();
      total_lookup_time += duration_cast<dsec>(end - start).count();
      #else
      best = ht_lookup(H, key);
      #endif
    }

    if(best != EMPTY) {
      jobs.pop_back();
      if(best == INT_MAX) {
        *jobs[curr_idx].write_result = best;
      } else {
        *jobs[curr_idx].write_result += best;
      }
       //notice += here
      if(jobs[curr_idx].write_result == &output_result) {
        break;
      }
      continue;
    }

    /* Add new job */
    #ifdef TIME_CODE
    auto start = Clock::now();
    #endif
    jobs[curr_idx].waiting_for_results = true;
    jobs[curr_idx].results.resize(G->parents[curr_v].size()+1);
    
    vector<int> indices;
    for(size_t i = 0 ; i < jobs[curr_idx].results.size() ; i++) {
      indices.push_back((int)i);
    }
    shuffle(indices.begin(), indices.end(), rng);

    for(size_t b = 0 ; b < indices.size() ; b++) {
      int idx = indices[b];

      jobs.push_back(job_t());
      jobs.back().problem.k = curr_k-1;
      jobs.back().write_result = &jobs[curr_idx].results[idx];
      jobs.back().waiting_for_results = false;
      if(idx == 0) {
        jobs.back().problem.v = curr_v;
        jobs[curr_idx].results[idx] = 0;
      } else {
        jobs.back().problem.v = G->parents[curr_v][idx-1].u;
        jobs[curr_idx].results[idx] = G->parents[curr_v][idx-1].weight;
      }
    }

    #ifdef TIME_CODE
    auto end = Clock::now();
    total_job_time += duration_cast<dsec>(end - start).count();
    #endif
  }
  #ifdef TIME_CODE
  printf("ht_lookup: %lf & job add: %lf & ht_insert %lf\n", total_lookup_time, total_job_time, total_insertion_time);
  #endif
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

void *thread_routine(void *in_args) {
  thread_arg_t *args = (thread_arg_t *)in_args;
  pthread_mutex_lock(args->mutex_start);
  while(!*args->start) {
    pthread_cond_wait(args->cond_start, args->mutex_start);
  }
  pthread_mutex_unlock(args->mutex_start);
  int res = bellman(args->H, args->G, args->source, args->dest);
  
  *(args->write_result) = res;
  pthread_mutex_lock(args->mutex);
  *args->done_thread = args->thread_id;
  pthread_mutex_unlock(args->mutex);
  pthread_cond_signal(args->cond);

  return (void *)0;
}

void read_test_case(char *input_filename, graph_t *G, int *source, int *dest, int *refsol) {
  FILE *input = fopen(input_filename, "r");

  if (input == NULL) {
    printf("Failed to open file %s\n", input_filename);
    return;
  }

  // Read in capacity and num_objects
  fscanf(input, "%d %d\n", &G->V, &G->E);
  fscanf(input, "%d %d\n", source, dest);

  G->parents.resize(G->V);

  // Read in values
  int u,v,w;
  for (int i = 0; i < G->E; i++) {
    fscanf(input, "%d %d %d\n", &u, &v, &w);
    G->parents[v].push_back(edge_t());
    G->parents[v].back().u = u;
    G->parents[v].back().v = v;
    G->parents[v].back().weight = w;
  }

  // Read in the reference solution
  fscanf(input, "%d\n", refsol);

  // Close file
  fclose(input);
}

int main(int argc, char *argv[]) {
  int opt;
  int num_threads = 1;
  char *input_filename = NULL;
  do {
    opt = getopt(argc, argv, "n:f:");
    switch (opt) {
    case 'f':
      input_filename = optarg;
      break;
    case 'n':
      num_threads = atoi(optarg);
      break;
    case -1:
      break;
    default:
      break;
    }
  } while (opt != -1);

  assert(input_filename != NULL);

  graph_t G;
  int source;
  int dest;
  int refsol;

  read_test_case(input_filename, &G, &source, &dest, &refsol);

  htable_t H = ht_new(pow(2,30), murmur_hash);
  
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  pthread_cond_t cond_start;
  pthread_mutex_t mutex_start;
  int done_thread = -1;
  assert(pthread_cond_init(&cond, NULL) == 0);
  assert(pthread_mutex_init(&mutex, NULL) == 0);
  assert(pthread_cond_init(&cond_start, NULL) == 0);
  assert(pthread_mutex_init(&mutex_start, NULL) == 0);

  bool should_start = false;

  // TIME ENTRY

  // Spawn threads to all compute the knapsack problem
  vector<pthread_t> threads(num_threads);
  thread_arg_t *td = new thread_arg_t[num_threads];
  int *results = new int[num_threads];
  for (int i = 0; i < num_threads; i++) {
    td[i].cond = &cond;
    td[i].mutex = &mutex;
    td[i].cond_start = &cond_start;
    td[i].mutex_start = &mutex_start;
    td[i].start = &should_start;
    td[i].done_thread = &done_thread;
    td[i].thread_id = i;
    td[i].H = H;
    td[i].write_result = &results[i];
    td[i].G = &G;
    td[i].source = source;
    td[i].dest = dest;
    pthread_create(&threads[i], NULL, thread_routine, &td[i]);
  }

  printf("Starting computation\n");
  pthread_mutex_lock(&mutex_start);
  auto start = std::chrono::high_resolution_clock::now();
  should_start = true;
  pthread_mutex_unlock(&mutex_start);
  pthread_cond_broadcast(&cond_start);

  // Wait for one thread to finish execution
  int final_result;
  int final_thread_done;
  pthread_mutex_lock(&mutex);
  while(done_thread == -1) {
    pthread_cond_wait(&cond, &mutex);
  }

  final_thread_done = done_thread;
  final_result = results[done_thread];
  pthread_mutex_unlock(&mutex);

  // TIME EXIT
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> float_ms = end - start;

  // Printing results
  printf("Reference Solution: %d\n", refsol);
  printf("thread %d result: %d in %lf milliseconds\n", final_thread_done, final_result, float_ms.count());
  assert(refsol == final_result);
  printf("Results matched!\n");

  // Cleanup
  ht_free(H);
  delete [] results;
  pthread_cond_destroy(&cond);
  pthread_mutex_destroy(&mutex);
  return 0;
}
