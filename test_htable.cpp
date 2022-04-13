#include "htable.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>
#include <pthread.h>
#include <thread>

/* TEST SUITE FOR HASH TABLES

Actions to test:
- Inserting a key
- Looking up the value of a key
- Multi-threaded correctness (w/ reference to Dynamic Programming)

Compiler/Linking Dependencies:
- must add this flag for pthreads -lpthread
- must add this flag for atomics  -std=c++11
- compilation: % g++ -std=c++11 -lpthread -o test_htable test_htable.cpp htable.cpp
*/

#define MAX_KEY 1000

enum Instruction { INSERT, LOOKUP };

struct thread_data {
  htable_t H;
  vector<int> test_keys;
};

Instruction random_instruction() {
  if (rand() % 2 == 0) {
    return INSERT;
  } 
  return LOOKUP;
}

void generate_instructions(vector<Instruction> &instructions) {
  generate(instructions.begin(), instructions.end(), random_instruction);
}

int random_key() {
  return rand() % MAX_KEY + 1;
}

void generate_keys(vector<int> &test_keys) {
  generate(test_keys.begin(), test_keys.end(), random_key);
}

// performs n/d but rounds up
int UPDIV(int n, int d) {
    return (n+d-1)/d;
}

void test_htable_single_threaded(htable_t H, int num_instructions) {
  map<int, int> reference_table;
  vector<Instruction> instructions (num_instructions);
  vector<int> test_keys (num_instructions);

  generate_instructions(instructions);
  generate_keys(test_keys);

  for (uint32_t i = 0; i < instructions.size(); i++) {
    switch(instructions[i]) {
      case INSERT:
        // printf("INSERTING %d\n", test_keys[i]); 
        ht_insert(H, test_keys[i], test_keys[i]);
        assert(ht_lookup(H, test_keys[i]) == test_keys[i]);
        reference_table[test_keys[i]] = test_keys[i];
        break;
      case LOOKUP:
        // printf("LOOKING UP %d\n", test_keys[i]); 
        
        int v = ht_lookup(H, test_keys[i]);
        map<int,int>::iterator it = reference_table.find(test_keys[i]);
        if (it != reference_table.end()) {
          // printf("REFERENCE TABLE VALUE: %d\n", it->second);
          // printf("MY TABLE VALUE: %d\n", v);
          assert(it->second == v);
        } else {
          // printf("REFERENCE TABLE VALUE: EMPTY\n");
          // printf("MY TABLE VALUE: %d\n", v);
          assert(v == EMPTY);
        }
        break;
    }
  }
}

void append_vectors(vector<int> &v1, vector<int> &v2) {
  for (uint32_t i = 0; i < v2.size(); i++) {
    v1.push_back(v2[i]);
  }
}

void print_vector(vector<int> v) {
  printf("vector: ");
  for (uint32_t i = 0; i < v.size(); i++) {
    printf("%d ", v[i]);
  }  
  printf("\n");
}

vector<int> flatten(vector<vector<int> > &vec) {
  vector<int> flattened;
  for (uint32_t i = 0; i < vec.size(); i++) {
    flattened.insert(flattened.end(), vec[i].begin(), vec[i].end());
  }
  return flattened;
}

void *single_threaded_actions(void *td) {
  struct thread_data *data = (struct thread_data*)td;
  htable_t H = data->H;
  vector<int> test_keys = data->test_keys;
  // print_vector(test_keys);
  
  for (uint32_t i = 0; i < test_keys.size(); i++) {
    ht_insert(H, test_keys[i], test_keys[i]);
    // printf("INSERTING %d\n", test_keys[i]);
    assert(ht_lookup(H, test_keys[i]) == test_keys[i]);
  }
  pthread_exit(NULL);
}

void test_htable_multi_threaded(htable_t H, int num_inserts, int num_threads) {
  int chunk_size = UPDIV(num_inserts, num_threads);

  vector<int> thread_keys (chunk_size);
  vector<vector<int>> all_keys (num_threads, thread_keys);
  struct thread_data *td = new struct thread_data[num_threads];

  // Generate random keys to insert for each thread
  for (int i = 0; i < num_threads; i++) {
    generate_keys(all_keys[i]);
  }

  // Spawn threads for insertion of keys
  pthread_t *threads = new pthread_t[num_threads];
  for (int i = 0; i < num_threads; i++) {
    // Have each thread insert keys into the hash table
    td[i].H = H;
    td[i].test_keys = all_keys[i];
    pthread_create(&threads[i], NULL, single_threaded_actions, &td[i]);
  }

  // Wait for all threads to finish execution
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }
  // Free allocated thread data
  delete [] td;

  // Free allocated pthreads
  delete [] threads;

  // printf("ALL THREADS FINISHED INSERTION\n");

  // After all threads are done, assert that all keys have been inserted into the table
  vector<int> flattened = flatten(all_keys);
  for (uint32_t i = 0; i < flattened.size(); i++) {
    assert(ht_lookup(H, flattened[i]) == flattened[i]);
  }
}

inline static uint32_t murmur_hash(key h) {
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

int main(int argc, char *argv[]) {
  int num_threads = 1;
  int capacity = 5000;
  int opt = 0;
  int num_instructions = 2500;

  // Read command line arguments
  do {
    opt = getopt(argc, argv, "n:c:i:");
    switch (opt) {
    case 'n':
      num_threads = atoi(optarg);
      break;
    case 'c':
      capacity = atoi(optarg);
      break;
    case 'i':
      num_instructions = atoi(optarg);
      break;
    case -1:
      break;
    default:
      break;
    }
  } while (opt != -1);

  htable_t H = ht_new(capacity, &murmur_hash);

  if (num_threads == 1) {
    test_htable_single_threaded(H, num_instructions);
  } else {
    test_htable_multi_threaded(H, num_instructions, num_threads);
  }

  ht_free(H);

  printf("All test cases passed!\n");
}
