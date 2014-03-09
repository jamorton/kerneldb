
#include "user.h"
#include "db.h"

static KrUser * userlist = NULL;

KrUser * kr_user_get(int pid)
{
    for (KrUser * user = userlist; user != NULL; user = user->next)
        if (user->pid == pid)
            return user;

    KrUser * user = (KrUser *)kmalloc(sizeof(KrUser), GFP_KERNEL);
    user->pid = pid;
    user->db = NULL;
    user->next = userlist;
    userlist = user;
    return user;
}
