#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <stdint.h>
#include "datastructures/symboltable.h"
#include "datastructures/bitset.h"

/**
 * defines types as integers (same purpose as enum)
 * used instead of enum in order to avoid different memory allocation for enums 
 * on complile for different operating systems
 */
#define TYPE_UNDEFINED 0
#define TYPE_8 1
#define TYPE_16 2
#define TYPE_32 3
#define TYPE_64 4
#define TYPE_STRING 5







/**
 * @brief Union capable of storing different types of attribute data.
 */
typedef union{
    uint8_t char_u[8];
    uint16_t int_u[4];
    uint32_t long_u[2];
    uint64_t longlong_u;
}AttrUnion;


/**
 * @brief Attribute table, saves names and data associated with
 * elements. 
 */
typedef struct{
    uint32_t type;
    /* only used if TYPE_STRING */
    uint32_t stringLength;
    AttrUnion* data;
} Attributes;


/**
 * @brief Struct representing database.
 */
typedef struct{
    /* Bitset */
    bitset*      sets;
    /* holds attributes for elements (integers, strings...) */
    Attributes*  attributes;
    /* symbol table with name as key */
    SymbolTable* setNamesTable;
    /* attribute table with name as key to get index for that attribute table */
    SymbolTable* attrNamesTable;
    /* element table with name as key to get index for element with that key */
    SymbolTable* keyTable;

    /* max size and current size for the number of keys/elements in the DB */
    uint64_t                maxKeyCount;
    uint64_t                keyCount;

    /* max size and current amount of sets in database */
    uint64_t                maxSetSize;
    uint64_t                setCount;;

    /* max and current amout of attributes in database */
    uint64_t                maxAttrSize;
    uint64_t                attrCount;
} Database;


/**
 * @brief create an empty database of a certain size
 * 
 * @param size amount of memory to allocate
 * @param sets amount of set tables to allocate
 * @param attributes amount of attribute tables to allocate
 * @param elementNames holds strings for all element names. Name of element i should be at index i.
 * @return struct Database pointer with allocated memory
 */
Database* createEmptyDB(long universeSize, long setSize, long attrSize);



void db_removeFromSet(Database* db, char*set, char*key);

void db_createSet(Database* db, char*name);

void db_addToSet(Database* db, char* set, char* element);

void db_createAttribute(Database* db, char* name, int type, int stringSize);

void db_setAttribute(Database* db, char* attrName, char* keyName, void* data);

void db_addKey(Database* db, char*name);

void db_destroy(Database* db);

void db_print(Database* db);

void db_test(void);

#endif