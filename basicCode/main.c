#include "hash.c"
#include "database.h"

typedef struct fileData{
	void * firstBlock;
	size_t size;	
} fileData;

void *inderectMalloc(size_t size){
	return malloc(size);
}

int createDB(dataBase **db, char* name){
	*db = (dataBase*)inderectMalloc(sizeof(dataBase));
	(*db)->name = name;
	(*db)->size = 1111;
	(*db)->hashTable = ht_create(sizeof(fileData) * 64);
	if((*db)->hashTable != NULL)
		return 1;
	else
		return 0;
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
	if (temp == NULL)
		return 0;
	*data = temp->firstBlock;
	return 1;
}

int delete(dataBase *db, char *key){
	fileData *temp = (fileData *)ht_get(db->hashTable, key);
	db->size -= temp->size;
	ht_remove(db->hashTable, key);
	//free(temp); don't think it's needed, think it's taken care of by remove
}

size_t value_size(dataBase *db, char *key){
	fileData *temp = (fileData *)ht_get(db->hashTable, key);
	return temp->size;
}
int main(int argc, char * argv[]){
        HT *hashTable = ht_create(sizeof(fileData) * 64);
        dataBase *db;
        createDB(&db, "myDB");
        int temp = 0;
        while(temp != -1){
                char line[50];
                char data[100];
								char *key = (char*)inderectMalloc((size_t)(sizeof(char)*50));
								char *stringData;
                size_t size;
                printf("0 to Add key and data, 1 to get data, 2 to remove data, 3 for size, -1 for quit\n");
                if(fgets(line, sizeof(line), stdin)) {
                        if(1 == sscanf(line, "%d", &temp)){
                                switch(temp){
                                        case 0:
                                                printf("Enter key\n");
                                                fgets(line, sizeof(line), stdin);
                                                sscanf(line, "%s", key);
																								printf("Enter data\n");
                                                fgets(data, sizeof(data), stdin);
                                                put(db, key, data, sizeof(data));
                                                break;
                                        case 1:
                                                printf("Enter key\n");
                                                fgets(line, sizeof(line), stdin);
                                                sscanf(line, "%s", key);
																								get(db, key, (void**)&stringData);
                                                printf("Data is: %s \n", stringData);
                                                break;
                                        case 2:
                                                printf("Enter key\n");
                                                fgets(line, sizeof(line), stdin);
                                                sscanf(line, "%s", key);
																								delete(db, key);
                                                break;
                                        case 3:
                                                printf("Enter key\n");
					        fgets(line, sizeof(line), stdin);
																								sscanf(line, "%s", key);
                                                size = value_size(db, key);
                                                printf("Size is: %d\n", (int)size);
                                                break;
					}
				}
			}
		}
	
}	
