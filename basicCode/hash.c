#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>

#include "hash.h"

unsigned long _hash(char *key)
{
  /* djb2 */
  unsigned long hash = 5381;
  int c;
 
  while (c = *key++)
    hash = ((hash << 5) + hash) + c;

  return hash;
}

HT *ht_create(int size)
{
  HT *ht = malloc(sizeof(HT));

  ht->size = size;

  ht->tbl = calloc(1, size * sizeof(struct ht_node *));

  return ht;
}

void ht_destroy(HT *ht)
{
  if (!ht) return;

  int i;
  for (i = 0; i < ht->size; i++) {
    struct ht_node *n = ht->tbl[i];
    while (n) {
      struct ht_node *n_old = n;
      
      n = n->nxt;

      free(n_old->key);
      n_old->key = NULL;
      free(n_old);
      n_old = NULL;
    }
  }
 
  free(ht->tbl);
  free(ht);

  ht = NULL;
}

size_t ht_get_size(HT * ht, char * key){
  if (!ht) return -1;
  unsigned long idx = _hash(key) % ht->size;

  struct ht_node *n = ht->tbl[idx];
  while (n) {
    if (strncmp(key, n->key, HT_MAX_KEYLEN) == 0)
      return n->size;

    n = n->nxt;
  }

  return -1;
}

void *ht_get(HT *ht, char *key)
{
  if (!ht) return NULL;

  unsigned long idx = _hash(key) % ht->size;

  struct ht_node *n = ht->tbl[idx];
  while (n) {
    if (strncmp(key, n->key, HT_MAX_KEYLEN) == 0)
      return n->val;

    n = n->nxt;
  }

  return NULL;
}

void ht_put(HT *ht, char *key, void *val, size_t size)
{
  if (!ht) return;

  unsigned long idx = _hash(key) % ht->size;

  struct ht_node *n_new = calloc(1, sizeof(struct ht_node));
  n_new->val = val;
  n_new->size = size;
  n_new->key = calloc(1, strnlen(key, HT_MAX_KEYLEN) + 1);
  strcpy(n_new->key, key);
  
  n_new->nxt = ht->tbl[idx];
  ht->tbl[idx] = n_new;
}

void ht_remove(HT *ht, char *key)
{
  if (!ht) return;

  unsigned long idx = _hash(key) % ht->size;

  struct ht_node *p = NULL, *n = ht->tbl[idx];
  while (n) {
    if (strncmp(key, n->key, HT_MAX_KEYLEN) == 0) {
      if (p)
        p->nxt = n->nxt;
      
      free (n->key);
      n->key = NULL;

      if (ht->tbl[idx] == n)
        ht->tbl[idx] = NULL;

      free (n);
      n = NULL;

      break;
    }

    p = n;
    n = n->nxt;
  }
}
