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
 * Rehashes all old table entries to the new table
 * Note: Currently unused since table does not resize
 */
void reshash_entries(htable_t old_ht, htable_t new_ht) {
  for (uint32_t i = 0; i < old_ht->capacity; i++) {
    entry_t entry = old_ht->table[i].load();
    if (entry.k != EMPTY) {
      ht_insert(new_ht, entry.k, entry.v);
    }
  }
}

/* 
 * Checks if the table needs to be resized, 
 * and if so, resizes table to the next power of 2 
 * Note: Currently unused since table does not resize
 */
void perform_resize_if_necessary(htable_t H) {
  // TODO: change this to resize according to a load factor
  if (H->size == H->capacity) {
    // Increase table size to next power of two
    uint32_t bigger_capacity = H->capacity << 1;

    // Create new hash table for rehashing purposes
    htable_t new_ht = ht_new(bigger_capacity, H->hf);

    // Rehash all entries for the new table
    reshash_entries(H, new_ht);

    // Free old table
    delete [] H->table;

    // Assign new table and capacity appropriately for caller
    H->table = new_ht->table;
    H->capacity = new_ht->capacity;

    // Free newly allocated hash table for resizing purposes
    delete new_ht;
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
  htable_t H = new hash_table;
  H->capacity = next_pow_2(capacity);
  H->size = 0;
  H->table = new atomic<entry_t>[H->capacity];
  H->hf = hf;
  return H;
}

/*
 * Linearly probes through the table to find the associated value v for key k
 * If the key k is not found in the table, then EMPTY is returned
 * Note: Since the table does not resize, this function will loop infinitely if 
 *       looking for a key not in the table while the table is full
 */
value ht_lookup(htable_t H, key k) {
  for (int i = hash_key(H, k, H->capacity); ; i++) {
    i = i & (H->capacity-1);
    entry_t entry = H->table[i].load();
    if (entry.k == k) {
      return entry.v;
    }
    if (entry.k == EMPTY) {
      return EMPTY;
    }
  }
}

/*
 * Linearly probes through the table to insert key k in an EMPTY spot
 * Note: Assumes that each key has a unique value which is true for dynamic programming problems. 
 * Note: Table does not resize so table MUST be initialized with a large enough capacity
 */
void ht_insert(htable_t H, key k, value v) {
  for (int i = hash_key(H, k, H->capacity); ; i++) {
    i = i & (H->capacity-1);
    entry_t desired_entry = {k, v};
    entry_t expected_entry = {EMPTY, EMPTY};    
    if (H->table[i].compare_exchange_strong(expected_entry, desired_entry)) {
      return;
    }
  }
}

/*
 * Frees the overall hash table struct and array of entries
 */
void ht_free(htable_t H) {
  delete [] H->table;
  delete H;
}