#include "htable.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

/* TEST SUITE FOR HASH TABLES

Actions to test:
- Inserting a key
- Looking up the value of a key
- Multi-threaded correctness (w/ reference to Dynamic Programming)
*/

#define MAX_KEY 1000

enum Instruction { INSERT, LOOKUP };

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

void test_htable_single_threaded(htable_t H, vector<Instruction> &instructions, vector<int> &test_keys) {
  map<int, int> reference_table;

  for (int i = 0; i < instructions.size(); i++) {
    switch(instructions[i]) {
      case INSERT:
        printf("INSERTING %d\n", test_keys[i]); 
        ht_insert(H, test_keys[i], test_keys[i]);
        assert(ht_lookup(H, test_keys[i]) == test_keys[i]);
        reference_table[test_keys[i]] = test_keys[i];
        break;
      case LOOKUP:
        printf("LOOKING UP %d\n", test_keys[i]); 
        
        int value = ht_lookup(H, test_keys[i]);
        map<int,int>::iterator it = reference_table.find(test_keys[i]);
        if (it != reference_table.end()) {
          printf("REFERENCE TABLE VALUE: %d\n", it->second);
          printf("MY TABLE VALUE: %d\n", value);
          assert(it->second == value);
        } else {
          printf("REFERENCE TABLE VALUE: EMPTY\n");
          printf("MY TABLE VALUE: %d\n", value);
          assert(value == EMPTY);
        }
        break;
    }
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
  char *inputFilename = NULL;
  int num_threads = 1;
  int capacity = 256;
  int opt = 0;
  int num_instructions = 2500;

  // Read command line arguments
  do {
    opt = getopt(argc, argv, "f:n:c:i:");
    switch (opt) {
    case 'f':
      inputFilename = optarg;
      break;
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
  vector<Instruction> instructions (num_instructions);
  vector<int> test_keys (num_instructions);

  generate_instructions(instructions);
  generate_keys(test_keys);

  if (num_threads == 1) {
    test_htable_single_threaded(H, instructions, test_keys);
  }

  printf("All test cases passed!\n");
}