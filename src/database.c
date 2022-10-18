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
static void printSets(Database* db);
static void printAttributes(Database* db);
static int typeSize(int type);
static void printBits(int size, uint64_t value);







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


/**
 * @brief removes a key from a set 
 * Switches a bit to 0 at the index of the key given by the keyTable in the database
 * @param db Database to use
 * @param set name of the set
 * @param key name of the key to remove
 */
void db_removeFromSet(Database* db, char*set, char*key){
    printf("db_removeFromSet \t- removing %s from set %s\n", key, set);
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


/**
 * @brief Adds one element to the set 
 * Switches one bit in the set to a 1 on the index corresponding to the element.
 * @param db Database to use
 * @param set name of the set to add element to
 * @param key name of the key
 */
void db_addToSet(Database* db, char* set, char* key){
    printf("db_addToSet \t- adding %s to set %s\n", key, set);
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

    db->sets[sdata->index][index2] |= (uint64_t)1 << index;
}

/**
 * @brief Adds a new key to the database. Can't have the same name as another
 * key.
 * 
 * @param db Database to add key to
 * @param name Name of the key
 */
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
 * If type is TYPE_UNDEFINED the data block wont be allocated.
 * Amount of elements in data depends on how many elements each attribute union can hold.
 * @param db database to create attribute table in
 * @param name name of the new table
 * @param type the datatype of the attributes to be stored
 * @param stringSize optional argument. Defines the maximum size of strings.
 */
void db_createAttribute(Database* db, char* name, int type, int stringSize){
    printf("db_createAttribute \t- name %s, type %d, with string size %d", name, type, stringSize);

    if(type == TYPE_STRING && stringSize < 1){
        errno = EINVAL;
        perror("db_createAttribute \t- type is TYPE_STRING but stringSize is < 1");
        return;
    }

    size_t numberOfElements = 0;

    switch (type)
    {
    case TYPE_8:        numberOfElements = db->keyCount/8;  break;
    case TYPE_STRING:   numberOfElements = db->keyCount/8*stringSize;  
                        db->attributes[db->attrCount].stringLength = stringSize;
                        break;
    case TYPE_16:       numberOfElements = db->keyCount/4;  break;
    case TYPE_32:       numberOfElements = db->keyCount/2;  break;
    case TYPE_64:       numberOfElements = db->keyCount;    break;
    default: break;
    }
    db->attributes[db->attrCount].type = type;
    db->attributes[db->attrCount].data = calloc(numberOfElements, sizeof(AttrUnion));
    st_insert(db->attrNamesTable, name, db->attrCount);
    db->attrCount++;
}




void db_setAttribute(Database* db, char* attrName, char* keyName, void* data){
    TableData* attrData = st_search(db->attrNamesTable, attrName);
    long attrIndex = attrData->index;

    TableData* keyData = st_search(db->keyTable, keyName);
    long keyIndex = keyData->index;
    int type = db->attributes[attrIndex].type;

    switch (type)
    {
    case TYPE_8:        db->attributes[attrIndex].data[keyIndex/8].char_u[keyIndex%8] = *((uint8_t*) data);
                        break;

    case TYPE_STRING:   for (size_t i = 0; i < db->attributes[attrIndex].stringLength; i++){
                            db->attributes[attrIndex].data[(keyIndex+i)/8].char_u[(keyIndex+i)%8] = *((uint8_t*) data);
                        }
                        break;

    case TYPE_16:       db->attributes[attrIndex].data[keyIndex/4].int_u[keyIndex%4] = *((uint16_t*) data); 
                        break;

    case TYPE_32:       db->attributes[attrIndex].data[keyIndex/2].long_u[keyIndex%2] = *((uint32_t*) data); 
                        break;

    case TYPE_64:       db->attributes[attrIndex].data[keyIndex].longlong_u = *((uint64_t*) data);
                        break;
    default: break;
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
    printAttributes(db);
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


        long b = db->keyCount / (typeSize(attr.type)*8)+1;


        for (long j = 0; j < b; j++){
            AttrUnion k = attr.data[j];
            switch (attr.type)
            {
            case TYPE_8:
                printBits(typeSize(attr.type), (uint64_t) k.char_u);
                break;
            
            default:
                break;
            }
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


/**
 * @brief Converts a type to the size of that type in bytes
 * Use the defines in the database.h file.
 * 
 * @param type type to get size of
 * @return int size of the type, or -1 if string or undefined.
 */
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