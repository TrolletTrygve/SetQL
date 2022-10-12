#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>		/* for CHAR_BIT */
#include <errno.h>
#include "datastructures/database.h"

#define WORD_BITS (8 * sizeof(unsigned int));
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

/* local functions */
int findFirstEmptySetIndex(Database* db);
void printSets(Database* db);
void printAttributes(Database* db);
int typeSize(int type);
void printBits(int size, uint64_t value);








/**
 * @brief create an empty database of a certain size
 * 
 * @param size amount of memory to allocate
 * @param sets amount of set tables to allocate
 * @param attributes amount of attribute tables to allocate
 * @return struct Database pointer with allocated memory
 *
 * TODO: write description
 * TODO: handle when user want's to increase size of database (setSize/attrSize)
 */
Database* createEmptyDB(long maxUniverseSize, long maxSetSize, long maxAttrSize){
    printf("createEmptyDB \t- allocate memory for database and symbol table\n");
    Database* db = malloc(sizeof(Database));
    db->setNamesTable = create_table(maxSetSize*1.3);
    db->attrNamesTable = create_table(maxAttrSize*1.3);
    db->keyTable = create_table(maxUniverseSize*1.3);

    printf("createEmptyDB \t- store attributes\n");
    db->maxKeyCount =  maxUniverseSize;
    db->maxSetSize = maxSetSize;
    db->maxAttrSize = maxAttrSize;

    db->keyCount = 0;
    db->setCount = 0;
    db->attrCount = 0;

    printf("createEmptyDB \t- initializes set\n");
    db->sets = malloc(sizeof(uint64_t*)*maxSetSize);
    printf("createEmptyDB \t- initializes attr size %ld\n", sizeof(Attributes)*maxAttrSize);
    db->attributes = malloc(sizeof(Attributes)*maxAttrSize);

    printf("createEmptyDB \t- done\n");

    return db;
}



/**
 * @brief find first free index among sets in database
 * 
 * @param db 
 * @return int index, or -1 if no index was found
 */
int findFirstEmptySetIndex(Database* db){
    for (long i = 0; i < db->setCount; i++){
        if(db->sets[i] == NULL){
            return i;
        }
    }

    return -1;
}

/**
 * @brief adds new set to the database initialized with 0
 * 
 * @param db database to add set to
 * @param name name of set used to retrieve the set
 * @return void* 
 * 
 * TODO: handle when user adds more sets than the database has capacity for
 */
void db_createSet(Database* db, char*name){
    if(strlen(name) < 1){
        fprintf(stderr, "db_createSet \t- ERROR! %s needs to be above 0 character long\n", name);
        return;
    }
    printf("db_createSet \t- adding set '%s'\n", name);
    int index = db->setCount;
    printf("db_createSet \t- insert set at set index %d\n", index);
    if(index >= db->maxSetSize){
        fprintf( stderr, "db_createSet \t- ERROR! Limit of amount of sets reached!\n");
        exit(0);
    }
    printf("\t\t- st_insert\n");
    st_insert(db->setNamesTable, name, index);
    printf("\t\t- calloc\n");
    db->sets[index] = calloc(db->maxKeyCount/8+1, 1);
    db->setCount++;
    printf("\t\t- done!\n");
}


void db_removeFromSet(Database* db, char*set, char*key){
    TableData* edata = st_search(db->keyTable, key);

    if(edata == NULL){
        perror("addToSet \t- ERROR! key name not in database");
        return;
    }

    TableData* sdata = st_search(db->setNamesTable, set);

    if(sdata == NULL){
        perror("addToSet \t- ERROR! set name not in database symbol table");
        return;
    }

    if(db->sets[sdata->index] == NULL){
        perror("addToSet \t- ERROR! set not in database");
        return;
    }
    long index = edata->index % (sizeof(uint64_t)*8);
    long index2 = edata->index / (sizeof(uint64_t)*8);
    db->sets[sdata->index][index2]&= ~(1 << index);

}



void db_addToSet(Database* db, char* set, char* key){
    printf("db_addToSet \t- adding %s to set %s", key, set);
    TableData* edata = st_search(db->keyTable, key);
    if(edata == NULL){
        errno = EINVAL;
        perror("addToSet \t- ERROR! key name not in database");
        return;
    }
    TableData* sdata = st_search(db->setNamesTable, set);
    if(sdata == NULL){
        errno = EINVAL;
        perror("addToSet \t- ERROR! set name not in database symbol table");
        return;
    }
    if(db->sets[sdata->index] == NULL){
        errno = EINVAL;
        perror("addToSet \t- ERROR! set not in database");
        return;
    }
    long index = edata->index % (sizeof(uint64_t)*8);
    long index2 = edata->index / (sizeof(uint64_t)*8);
    printf("\n\n");
    printBits(64, 1<<10);
    printf("\n\n");

    db->sets[sdata->index][index2] |= (uint64_t)1 << index;
}

void db_addKey(Database* db, char*name){
    printf("db_addKey \t- adding key '%s'\n", name);
    if(strlen(name) < 1){
        errno = EINVAL;
        perror("db_addKey \t- name of key must be longer than 0");
        return;
    }
    st_insert(db->keyTable, name, db->keyCount);
    db->keyCount++;
}

/**
 * @brief creates a new attribute table
 * @param db database to create attribute table in
 * @param name name of the new table
 * @param type the datatype of the attributes to be stored
 * @param stringSize optional argument. Defines the maximum size of strings.
 */
void db_createAttribute(Database* db, char* name, int type, int stringSize){
    if(type == TYPE_STRING && stringSize < 1){
        errno = EINVAL;
        perror("db_createAttribute \t- type is TYPE_STRING but stringSize is < 1");
        return;
    }

    db->attributes[db->attrCount].type = type;
    int size = typeSize(type);
    if(size > 0){
        db->attributes[db->attrCount].data = calloc(db->keyCount, size);
        st_insert(db->attrNamesTable, name, db->attrCount);
        db->attrCount++;
    }
}



/**
 * @brief free all allocated memory of a Database 
 * 
 * @param db Database struct to free
 * TODO: free all data here!
 */
void db_destroy(Database* db){
    free(db);
}



void db_print(Database* db){
    printf("\ndatabase\n");
    printf("----------------------------------\n");
    printf("universe size: %ld\n", db->keyCount);
    printf("setAmount size: %ld\n", db->setCount);
    printf("attrAmount size: %ld\n", db->attrCount);
    printSets(db);
    printAttributes(db);
    printf("----------------------------------\n\n");
}

void printSets(Database* db){
    printf("\n");
    print_table(db->setNamesTable);

    char** keys = st_getKeys(db->setNamesTable);
    int i = 0;
    while (keys[i] != NULL){

        TableData* data = (TableData*)st_search(db->setNamesTable, keys[i]);
        free(keys[i]);
        int index = data->index;
        uint64_t* set = db->sets[index];

        printf("set %s\n[", keys[i]);
        
        long b = db->keyCount / (sizeof(uint64_t)*8)+1;

        for (long j = 0; j < b; j++){
            printf("\n");
            uint64_t k = set[j];
            printBits(sizeof(uint64_t),k);
        }
        printf("\n]\n");
        i++;
    }

    printf("\n");    
    free(keys);
}

void printAttributes(Database* db){

    char** keys = st_getKeys(db->attrNamesTable);
    int i = 0;

    while (keys[i] != NULL){

        TableData* data = (TableData*)st_search(db->attrNamesTable, keys[i]);
        int index = data->index;
        Attributes attr = db->attributes[index];

        printf("\nAttribute %s\n[", keys[i]);

        for (long j = 0; j < db->keyCount; j++){
            long b = j % typeSize(attr.type);
            AttrUnion k = attr.data[b];
            printf("%hhn,", k.char_u);
        }
        printf("]\n");
        i++;
    }
}


void printBits(int size, uint64_t value){

    for(int i = 0;i<size*8;++i){
    // print last bit and shift left.
        printf("%u",value&1 ? 1 : 0);
        value = value>>1;
    }
}

int typeSize(int type){
    switch (type)
    {
    case TYPE_UNDEFINED: return -1;
    case TYPE_8: return 8;
    case TYPE_16: return 16;
    case TYPE_32: return 32;
    case TYPE_64: return 64;
    case TYPE_STRING: return -1;
    default: return -1;
    }
}