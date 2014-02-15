#include "hash.c"
#include "database.h"

typedef struct fileData{
	void * firstBlock;
	size_t size;	
} fileData;

void *inderectMalloc(size_t size){
	return malloc(size);
}

int main(int argc, char * argv[]){
	
}

int createDB(dataBase *db, char* name){
	db = inderectMalloc(sizeof(dataBase));
	db->name = name;
	db->size = 0;
	db->hashTable = ht_create(sizeof(fileData) * 64);
}

int put(dataBase *db, char *key, void * val, size_t dataSize){
	fileData * temp = (fileData *)inderectMalloc(sizeof(fileData));
	temp->firstBlock = val;
	temp->size = dataSize;
	db->size += dataSize;
	ht_put(db->hashTable, key, temp);
	return 1;
}

int get(dataBase *db, char *key, void ** data){
	fileData *temp = (fileData *)ht_get(db->hashTable, key);
	*data = temp->firstBlock;
	return 1;
}

int delete(dataBase *db, char *key){
	fileData *temp = (fileData *)ht_get(db->hashTable, key);
	db->size -= temp->size;
	ht_remove(db->hashTable, key);
}

size_t value_size(dataBase *db, char *key){
	fileData *temp = (fileData *)ht_get(db->hashTable, key);
	return temp->size;
}
