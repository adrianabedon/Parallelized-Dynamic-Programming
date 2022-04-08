#include "htable.h"

/* HELPER FUNCTIONS */

/* 
 * Rounds to the next power of 2 
 */
uint32_t next_pow_2(uint32_t n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

/* 
 * Checks if the table needs to be resized, 
 * and if so, resizes table to the next power of 2 
 */
void perform_resize_if_necessary(htable_t H) {
  if (H->size == H->capacity) {
    // Increase table size to next power of two
    uint32_t bigger_capacity = H->capacity << 1;

    // Allocate bigger table
    entry_t *bigger_table = (entry_t*)calloc(sizeof(entry_t), bigger_capacity);

    // Copy all bytes from previous table to new table
    memcpy(bigger_table, H->table, H->capacity * sizeof(entry_t));

    // Free old table
    free(H->table);

    // Assign new table and capacity appropriately
    H->table = bigger_table;
    H->capacity = bigger_capacity;
  }
}

/*
 * Hashes key k with the hash function provided by the client
 */
int hash_key(htable_t H, key k) {
  return (*(H->hf))(k) & (H->capacity-1);
}

/* INTERFACE FUNCTIONS */

/*
 * Returns a new hash table with the initial capacity rounded up to the next power of 2
 */
htable_t ht_new(uint32_t capacity, hash_fn hf) {
  htable_t H = (htable_t)malloc(sizeof(struct hash_table));
  H->capacity = next_pow_2(capacity);
  H->size = 0;
  H->table = (entry_t*)calloc(sizeof(entry_t), H->capacity);
  H->hf = hf;
  return H;
}

/*
 * Linearly probes through the table to find the associated value v for key k
 * If the key k is not found in the table, then EMPTY (-1) is returned
 */
value ht_lookup(htable_t H, key k) {
  for (int i = hash_key(H, k); ; i++) {
    i = i & (H->capacity-1);
    if (H->table[i].k == k) {
      return H->table[i].v;
    }
    if (H->table[i].k == EMPTY) {
      return EMPTY;
    }
  }
}

/*
 * Linearly probes through the table to insert key k in an EMPTY spot
 * If the key k is already in the table, the previously associated value is overrided with v
 * Additionally, the table resizes if table is already full.
 */
void ht_insert(htable_t H, key k, value v) {
  for (int i = hash_key(H, k); ; i++) {
    i = i & (H->capacity-1);
    if (H->table[i].k == EMPTY) {
      H->size++;
      H->table[i].k = k;
      H->table[i].v = v;
      perform_resize_if_necessary(H);
      return;
    }
    if (H->table[i].k == k) {
      H->table[i].v = v;
      return;
    }
  }
}
