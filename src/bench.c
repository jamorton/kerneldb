
/**
 * benchmarks that run in-kernel-module (instead of from userspace thru netlink)
 */

void kr_bench()
{
    KrDb* db;
    kr_db_open(&db, "/dev/sdb");





    kr_db_close(db);
}
