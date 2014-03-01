
/* simple hashtable
 * David Kaplan <david[at]2of1.org>, 2011
 *
 * mem = (16 + 8 * size + 24 * entries) bytes [64-bit]
 * mem = (8 + 4 * size + 12 * entries) bytes [32-bit]
 *
 * some sizes for reference [64-bit]
 * 2^8  = 16K [probably too small - depending on use]
 * 2^16 = 512K [even bigger would be better]
 * 2^24 = 128MB [probably a bit too much]
 * 2^32 = 32GB [whooooa! watch that heap space!]
 *
 * the hashtable uses basic linked-lists for handling collisions
 */


#define HT_MAX_KEYLEN 50

struct ht_node {
  void *val;
  size_t size;
  char *key;
  struct ht_node *nxt;
};



typedef struct ht {
  struct ht_node **tbl;
  int size;
} HT;

HT *ht_create(int size);                    /* allocate hashtable mem */
void ht_destroy(HT *ht);                    /* free hashtable mem */
void *ht_get(HT *ht, char *key);            /* retrieve entry */
void ht_put(HT *ht, char *key, void *val, size_t size);  /* store entry */
void ht_remove(HT *ht, char *key);          /* remove entry */
size_t ht_get_size(HT* ht, char *key);
