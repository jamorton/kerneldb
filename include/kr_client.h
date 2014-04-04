
#ifndef KR_CLIENT_H
#define KR_CLIENT_H

#include <stddef.h>

struct KrClient;
typedef struct KrClient KrClient;

KrClient *  kr_open  (const char * dev);
void        kr_close (KrClient* client);
void        kr_put   (KrClient* client, size_t keysz, void* key, size_t valsz, void* val);
void        kr_get   (KrClient* client, size_t keysz, void* key, size_t* valszout, void** valout);

#endif
