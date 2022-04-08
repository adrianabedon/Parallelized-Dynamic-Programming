#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define EMPTY 0

using namespace std;

typedef struct hash_table *htable_t;
typedef struct table_entry entry_t;

// keys and values are primitive types
typedef int key;
typedef int value;

typedef uint32_t (*hash_fn) (key);

struct table_entry {
  key k;
  value v;
};

struct hash_table {
  uint32_t capacity;
  uint32_t size;
	entry_t *table;
  hash_fn hf;
};

htable_t ht_new(uint32_t capacity, hash_fn hf);
//TODO: @requires capacity is a power of 2? or we can round up ourselves
//@ensures \result != NULL;

int ht_lookup(htable_t H, key k);
//@requires H != NULL;
//@ensures \result == EMPTY || \result >= 0;

void ht_insert(htable_t H, key k, value v);
//@requires H != NULL;

#endif


/* RESOURCES REFERENCED:

https://preshing.com/20130605/the-worlds-simplest-lock-free-hash-table/
https://web.stanford.edu/class/ee380/Abstracts/070221_LockFreeHash.pdf
https://github.com/aappleby/smhasher
*/
