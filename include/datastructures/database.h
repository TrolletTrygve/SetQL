
#include <stdint.h>
#include "datastructures/symbolTable.h"
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





union AttrUnion{
    int8_t char_u[8];
    int16_t int_u[2];
    int64_t long_u;
};


typedef struct{
    uint64_t type;
    union AttrUnion* data;
} Attributes;


/**
 * @brief Struct representing database
 */
typedef struct{
    uint64_t**   sets;
    Attributes*  attributes;
    SymbolTable* setNamesTable;
    SymbolTable* attrNamesTable;
    SymbolTable* elemNamesTable;

    long                universeSize;
    long                setAmount;
    long                attrAmount;
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
Database* createEmptyDB(long universeSize, long setSize, long attrSize, char** elementNames);

void addEmptySet(Database* db, char*name);

void addToSet(Database* db, char* set, char* element);

void destroyDB(Database* db);

void printDB(Database* db);