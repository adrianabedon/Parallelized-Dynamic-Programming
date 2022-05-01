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
 
void reshash_entries(htable_t old_ht, htable_t new_ht) {
  for (int i = 0; i < old_ht->capacity; i++) {
    entry_t entry = old_ht->table[i].load();
    if (entry.k != EMPTY) {
      ht_insert(new_ht, entry.k, entry.v);
    }
  }
}


 * Checks if the table needs to be resized, 
 * and if so, resizes table to the next power of 2 
 * Note: Currently unused since table does not resize

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
*/

/*
 * Hashes key k with the hash function provided by the client
 */
int hash_key(htable_t H, key_t k, uint32_t capacity) {
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
	H->table = new entry_t[H->capacity];

	// Initialize table with EMPTY keys and values
	for (uint32_t i = 0; i < H->capacity; i++) {
	  H->table[i].k.store(EMPTY);
		H->table[i].v = EMPTY;
	}

  H->hf = hf;
  return H;
}

/*
 * Returns the index of the entry with key k or the next EMPTY entry
 * Note: If table is full, returns TABLE_FULL
 */
int get_index_entry(htable_t H, key_t k) {
	int i = hash_key(H, k, H->capacity);
	entry_t *entry = &(H->table[i]);
	// Probes from the beginning of the hash
	uint32_t probes = 0;
	while (probes < H->capacity - 1 && entry->k.load() != k && entry->k.load() != EMPTY) {
	  probes += 1;
		i = (i + 1) & (H->capacity-1);
		entry = &(H->table[i]);
	}
	if (probes >= H->capacity-1) 
	  return TABLE_FULL;
	else
    return i;
}

/*
 * Linearly probes through the table to insert key k in an EMPTY spot
 * Note: Assumes that each key has a unique value which is true for dynamic programming problems
 * Note: Table does not resize so table MUST be initialized with a large enough capacity
 * Note: If two threads try to insert the same value, get_index_entry reprobes the table to avoid duplicate keys
 */
void ht_insert(htable_t H, key_t k, value_t v) {
  while (true) {
    int i = get_index_entry(H, k);
	
	  if (i == TABLE_FULL) {
	    printf("ERROR: hash table full\n");
	    return;
	  }

	  entry_t *entry = &(H->table[i]);
		key_t key_in_table = entry->k.load();
		
		// Overwrite value if key in table is the key we're inserting
		if (key_in_table == k) {
		  entry->v = v;
			return;
		}

		// If key in table is empty, check if we can swap in the new key
	  if (key_in_table == EMPTY) {
			key_t empty = EMPTY;
	    if (atomic_compare_exchange_strong(&(entry->k), &empty, k)) {
				// Swap succeeded! Insert corresponding value
		    entry->v = v;
			  return;
		  }
			// Swap failed. Another thread beat us, try again with the next entry.
	  }
	}
}

/*
 * Linearly probes through the table to find the associated value v for key k
 * If the key k is not found in the table, then EMPTY is returned
 * Note: Since the table does not resize, this function will loop infinitely if 
 *       looking for a key not in the table while the table is full
 */
value_t ht_lookup(htable_t H, key_t k) {
  for (int i = hash_key(H, k, H->capacity); ; i++) {
    i = i & (H->capacity-1);
    entry_t *entry = &(H->table[i]);
		key_t key_in_table = entry->k.load();
    if (key_in_table == k) {
      return entry->v;
    }
    if (key_in_table == EMPTY) {
      return EMPTY;
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
