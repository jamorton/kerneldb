
#ifndef KR_CLIENT_H
#define KR_CLIENT_H

struct KrClient;
typedef struct KrClient KrClient;

KrClient *  kr_open  (const char * dev);
void        kr_close (KrClient * client);

#endif
