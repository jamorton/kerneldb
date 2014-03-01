#include "hash.c"
#include "database.h"

size_t value_size(dataBase *, char*);

void *inderectMalloc(size_t size){
	return malloc(size);
}

int main(int argc, char * argv[]){
        dataBase *db;
        createDB(&db, "myDB");
        int temp = 0;
        while(temp != -1){
                char line[50];
                char data[100];
		char *key = (char*)inderectMalloc((size_t)(sizeof(char)*50));
		char *stringData;
                size_t size;
                printf("0 to Add key and data, 1 to get data, 2 to remove data, 3 for size, 4 for save, 5 for load, -1 for quit\n");
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
					case 4:
						printf("Enter path\n");
						fgets(line, sizeof(line), stdin);
						sscanf(line, "%s", key);
						if(-1 == saveHT(db, key))
							printf("Failed saved\n");
						break;
					case 5:
						printf("Enter path\n");
						fgets(line, sizeof(line), stdin);
						sscanf(line, "%s", key);
						if(-1 == loadHT(db, key))
							printf("Failed load\n");
						break;
				}
			}
		}
	}
}

int createDB(dataBase **db, char* name){
	*db = malloc(sizeof(dataBase));
	(*db)->name = name;
	(*db)->size = 0;
	(*db)->hashTable = ht_create(64);
}

int put(dataBase *db, char *key, void * val, size_t dataSize){
	ht_put(db->hashTable, key, val, dataSize);
	return 1;
}

int get(dataBase *db, char *key, void ** data){
	*data = (void *)ht_get(db->hashTable, key);
	return 1;
}

int delete(dataBase *db, char *key){
	ht_remove(db->hashTable, key);
}

int loadHT(dataBase *db, char *path){
	FILE * fpr;
	if((fpr = fopen(path, "r")) == NULL)
		return -1;
	long curSize = 0;
	fseek(fpr, 0, SEEK_END);
	long maxSize = ftell(fpr);
	rewind(fpr);
	while(curSize < maxSize){
		struct ht_node *n;
		fread(n, sizeof(struct ht_node), 1, fpr);
		void * temp = inderectMalloc(n->size);
		fread(temp, n->size, 1, fpr);
		put(db, n->key, temp, n->size);
		curSize = sizeof(struct ht_node) + n->size;
	}
	fclose(fpr);
	return 0;
}

int saveHT(dataBase *db, char *path){
	FILE * temp;
	if((temp = fopen(path, "w")) == NULL)
		return -1;
	HT * ht = db->hashTable;
	if(ht == NULL)
		return -1;
	int i;
	for(i = 0; i < ht->size; i++){
		struct ht_node *n = ht->tbl[i];
		while(n) {
			struct ht_node *n_old = n;
			n = n->nxt;
			fwrite(n_old, sizeof(struct ht_node), 1, temp);
			fwrite(n_old->val, n_old->size, 1, temp);	
		}
	}
	fclose(temp);
	return 0;
}

size_t value_size(dataBase *db, char *key){
	return ht_get_size(db->hashTable, key);
}
