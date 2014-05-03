
#include "internal.h"
#include "db.h"

#include <linux/time.h>

#if 0

#define NUM_ENTRIES 1000000

//calculate the difference between two times
int timeval_subtract(struct timeval * result, struct timeval *t2, struct timeval* t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;
    return diff < 0;
}

/**
 * benchmarks that run in-kernel-module (instead of from userspace thru netlink)
 */
void kr_bench(KrDb* db)
{
    char kbuf[50], vbuf[50];
    KrSlice out, key = {0, kbuf}, val = {0, vbuf};
    int i;
    struct timeval tvBegin, tvEnd, tvDiff;

    //put 1 million key-value pairs into a database and measure time
    do_gettimeofday(&tvBegin);
    for (i=0; i < NUM_ENTRIES; i++){
        key.size = sprintf(kbuf, "test %d",i);
        val.size = sprintf(vbuf, "val %d",i);
        if (kr_db_put(db, key, val) < 0)
            goto fail;
 	}
    do_gettimeofday(&tvEnd);
    timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
    printk(KERN_INFO "put time: %ld.%06ld\n", tvDiff.tv_sec, tvDiff.tv_usec);

    //get a million key-value pairs
    do_gettimeofday(&tvBegin);
    for(i = 0; i < NUM_ENTRIES; i++)
    {
        key.size = sprintf(kbuf,"test %d",i);
        if (kr_db_get(db, key, &out) < 0)
            goto fail;
    }
    do_gettimeofday(&tvEnd);
    timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
    printk(KERN_INFO "get time: %ld.%06ld\n", tvDiff.tv_sec, tvDiff.tv_usec);

    return;

 fail:
    printk(KERN_ERR "bench: ERROR OCCURED\n");
}

#endif

void kr_bench(KrDb* db)
{

}
