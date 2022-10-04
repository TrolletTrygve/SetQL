#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>		/* for CHAR_BIT */
#include "datastructures/database.h"


#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

/* local functions */
int findFirstEmptySetIndex(Database* db);


/**
 * @brief Database holds data for one database
 * TODO: write description
 * TODO: handle when user want's to increase size of database (setSize/attrSize)
 */
Database* createEmptyDB(long universeSize, long setSize, long attrSize){
    printf("createEmptyDB \t- allocate memory for database\n");
    Database* db = malloc(sizeof(Database));
    db->symbolTable = create_table(attrSize+setSize);

    printf("createEmptyDB \t- store attributes\n");
    db->universeSize =  universeSize;
    db->setAmount = setSize;
    db->attrAmount = attrSize;

    printf("createEmptyDB \t- initializes set\n");
    db->sets = malloc(sizeof(int*)*setSize);
    printf("createEmptyDB \t- initializes attr size %ld\n", sizeof(Attributes)*attrSize);
    db->attributes = malloc(sizeof(Attributes)*attrSize);

    printf("createEmptyDB \t- done\n");
    return db;
}

    /*for (long i = 0; i < setSize; i++){
        db->sets[i] = malloc(sizeof(int)*universeSize);
    }
    for (long i = 0; i < attrSize; i++){
        db->attributes[i].data = malloc(sizeof(Attributes));
        db->attributes->type=TYPE_UNDEFINED;
    }*/


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
    st_insert(db->symbolTable, name, index);
    db->sets[index] = calloc(db->universeSize, sizeof(int));
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
    print_table(db->symbolTable);
    char** keys = st_getKeys(db->symbolTable);
    int i = 0;
    while (keys[i] != NULL)
    {
        TableData* data = (TableData*)st_search(db->symbolTable, keys[i]);
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