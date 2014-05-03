
#ifndef KR_INTERNAL_H
#define KR_INTERNAL_H

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/slab.h>

typedef const char * kr_dataptr;

typedef u32 kr_hash;

#include "slice.h"
#include "error.h"

struct KrDb;
void kr_bench(struct KrDb* db);

#endif
