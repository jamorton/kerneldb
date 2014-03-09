
#ifndef KR_USER_H
#define KR_USER_H

typedef struct KrDb KrDb;

/**
 * User-space client
 */
typedef struct KrUser {
    int pid; /* user's process id (for netlink communication) */
    KrDb * db;
    struct KrUser next;
} KrUser;

KrUser * kr_get_user(int pid);

#endif
