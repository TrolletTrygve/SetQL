#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>		/* for CHAR_BIT */
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
void saveElementNames(Database* db, char** elementNames);






/**
 * @brief create an empty database of a certain size
 * 
 * @param size amount of memory to allocate
 * @param sets amount of set tables to allocate
 * @param attributes amount of attribute tables to allocate
 * @param elementNames holds strings for all element names. Name of element i should be at index i.
 * @return struct Database pointer with allocated memory
 *
 * TODO: write description
 * TODO: handle when user want's to increase size of database (setSize/attrSize)
 */
Database* createEmptyDB(long universeSize, long setSize, long attrSize, char** elementNames){
    printf("createEmptyDB \t- allocate memory for database and symbol table\n");
    Database* db = malloc(sizeof(Database));
    db->setNamesTable = create_table(setSize*1.3);
    db->attrNamesTable = create_table(attrSize*1.3);
    db->elemNamesTable = create_table(universeSize*1.3);

    printf("createEmptyDB \t- store attributes\n");
    db->universeSize =  universeSize;
    db->setAmount = setSize;
    db->attrAmount = attrSize;

    printf("createEmptyDB \t- initializes set\n");
    db->sets = malloc(sizeof(uint64_t*)*setSize);
    printf("createEmptyDB \t- initializes attr size %ld\n", sizeof(Attributes)*attrSize);
    db->attributes = malloc(sizeof(Attributes)*attrSize);

    printf("createEmptyDB \t- done\n");

    return db;
}


void saveElementNames(Database* db, char** elementNames){
    for (size_t i = 0; i < db->universeSize; i++){
            st_insert(db->elemNamesTable, elementNames[i], i);
    }
}


/**
 * @brief find first free index among sets in database
 * 
 * @param db 
 * @return int index, or -1 if no index was found
 */
int findFirstEmptySetIndex(Database* db){
    for (long i = 0; i < db->setAmount; i++){
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
void addEmptySet(Database* db, char*name){
    if(strlen(name) < 2){
        fprintf(stderr, "addEmptySet \t- ERROR: %s needs to be above 1 character long\n", name);
        return;
    }
    printf("addEmptySet \t- %s\n", name);
    int index = findFirstEmptySetIndex(db);
    printf("addEmptySet \t- insert set at set index %d\n", index);
    if(index == -1){
        fprintf( stderr, "ERROR: Limit of amount of sets reached!");
        exit(0);
    }
    st_insert(db->setNamesTable, name, index);
    db->sets[index] = calloc(db->universeSize, 1);
}



void addToSet(Database* db, char* set, char* element){
    TableData* edata = st_search(db, element);
    if(edata == NULL){
        perror("addToSet \t- ERROR, element name not in database");
        return;
    }
    TableData* sdata = st_search(db, set);
    if(sdata == NULL){
        perror("addToSet \t- ERROR, set name not in database symbol table");
        return;
    }
    if(db->sets[sdata->index] == NULL){
        perror("addToSet \t- ERROR, set not in database");
        return;
    }
    long index = edata->index % sizeof(uint64_t);
    db->sets[sdata->index][index] = db->sets[sdata->index][index] | 1 << edata->index/sizeof(uint64_t);
}



/**
 * @brief free all allocated memory of a Database 
 * 
 * @param db Database struct to free
 * TODO: free all data here!
 */
void destroyDB(Database* db){
    free(db);
}



void printDB(Database* db){

    printf("\ndatabase\n");
    printf("----------------------------------\n");
    printf("universe size: %ld\n", db->universeSize);
    printf("setAmount size: %ld\n", db->setAmount);
    printf("attrAmount size: %ld\n", db->attrAmount);
    print_table(db->setNamesTable);
    char** keys = st_getKeys(db->setNamesTable);
    int i = 0;
    while (keys[i] != NULL)
    {
        TableData* data = (TableData*)st_search(db->setNamesTable, keys[i]);
        int index = data->index;
        int* set = db->sets[index];
        printf("\nset %s\n[", keys[i]);
        for (long j = 0; j < db->universeSize; j++)
        {
            printf("%d,", set[j]);
        }
        printf("]\n");
        i++;
    }
    printf("----------------------------------\n\n");
}