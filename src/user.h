
#ifndef KR_USER_H
#define KR_USER_H

/**
 * User-space client
 */
typedef struct KrUser {
    int pid; /* user's process id (for netlink communication) */
    struct KrDb* db;
    struct KrUser* next;
} KrUser;

KrUser * kr_user_get(int pid);

#endif
