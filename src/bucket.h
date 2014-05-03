
#ifndef KR_BUCKET_H
#define KR_BUCKET_H

#include "internal.h"
#include "io.h"

#define KR_BUCKET_SIZE (KR_BLOCK_SIZE/4)
#define KR_BUCKETS_PER_BLOCK (KR_BLOCK_SIZE/KR_BUCKET_SIZE)

struct KrBuf;

void kr_bucket_init(char* data);
int kr_bucket_del(char* data, KrSlice* key);
int kr_bucket_get(char* data, KrSlice key, KrSlice* val);
int kr_bucket_add(char* data, KrSlice key, KrSlice val);

#endif
