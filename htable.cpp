#include "htable.h"
#include <cassert>

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

void reshash_entries(htable_t old_ht, htable_t new_ht) {
  for (int i = 0; i < old_ht->capacity; i++) {
    if (old_ht->table[i].k != EMPTY) {
      ht_insert(new_ht, old_ht->table[i].k, old_ht->table[i].v);
    }
  }
}

/* 
 * Checks if the table needs to be resized, 
 * and if so, resizes table to the next power of 2 
 */
void perform_resize_if_necessary(htable_t H) {
  // TODO: change this to resize according to a load factor
  if (H->size == H->capacity) {
    printf("====================== RESIZING ======================\n");
    // Increase table size to next power of two
    uint32_t bigger_capacity = H->capacity << 1;

    // Create new hash table for rehashing purposes
    htable_t new_ht = ht_new(bigger_capacity, H->hf);

    // Rehash all entries for the new table
    reshash_entries(H, new_ht);

    // Free old table
    free(H->table);

    // Assign new table and capacity appropriately for caller
    H->table = new_ht->table;
    H->capacity = new_ht->capacity;

    // Free newly allocated hash table for resizing purposes
    free(new_ht);
  }
}

/*
 * Hashes key k with the hash function provided by the client
 */
int hash_key(htable_t H, key k, uint32_t capacity) {
  return (*(H->hf))(k) & (capacity-1);
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
  uint32_t count = 0; 
  for (int i = hash_key(H, k, H->capacity); ; i++) {
    i = i & (H->capacity-1);
    if (H->table[i].k == k) {
      return H->table[i].v;
    }
    if (H->table[i].k == EMPTY) {
      return EMPTY;
    }
    // TODO: currently have a counter to check if key is not in table when table is full
    //       instead, should disallow the table being full by resizing when table exceeds 
    //       a load factor < 1
    if (count == H->capacity) {
      return EMPTY;
    }
    count++;
  }
}

/*
 * Linearly probes through the table to insert key k in an EMPTY spot
 * If the key k is already in the table, the previously associated value is overrided with v
 * Additionally, the table resizes if table is already full/exceeds the maximum load balance.
 */
void ht_insert(htable_t H, key k, value v) {
  perform_resize_if_necessary(H);

  for (int i = hash_key(H, k, H->capacity); ; i++) {
    i = i & (H->capacity-1);
    if (H->table[i].k == EMPTY) {
      H->size++;
      H->table[i].k = k;
      H->table[i].v = v;
      return;
    }
    if (H->table[i].k == k) {
      H->table[i].v = v;
      return;
    }
  }
}