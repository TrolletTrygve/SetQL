#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>		/* for CHAR_BIT */
#include "database.h"

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

/**
 * @brief Database holds data for one database
 * TODO: write description
 */
Database* createEmptyDB(long universeSize, long setSize, long attrSize){
    Database* db = malloc(sizeof(Database));

    db->universeSize =  universeSize;
    db->setAmount = setSize;
    db->attrAmount = attrSize;

    db->sets = malloc(sizeof(int*)*setSize);
    db->attributes = malloc(sizeof(Attributes)*attrSize);

    for (long i = 0; i < setSize; i++){
        db->sets[i] = malloc(sizeof(int)*universeSize);
    }
    for (long i = 0; i < attrSize; i++){
        db->attributes[i].data = malloc(sizeof(Attributes));
        db->attributes->type=TYPE_UNDEFINED;
    }

    return db;
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