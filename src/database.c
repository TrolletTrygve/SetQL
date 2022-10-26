/**
 * @file database.c
 * @author Simon Uttertröm (c18sum@cs.umu.se)
 * @brief Database management system structure and initialization for set operations.
 * @version 0.1
 * @date 2022-10-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>		/* for CHAR_BIT */
#include <errno.h>
#include "datastructures/database.h"
#include "utils.h"
#include <inttypes.h>


/* local functions */
static void printSets(Database* db);
static void printAttributes(Database* db);


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
    DEBUG_CALL(printf("createEmptyDB \t- allocate memory for database and symbol table\n"));
    Database* db = malloc(sizeof(Database));
    db->setNamesTable = create_table(maxSetSize*1.3);
    db->attrNamesTable = create_table(maxAttrSize*1.3);
    db->keyTable = create_table(maxUniverseSize*1.3);

    DEBUG_CALL(printf("createEmptyDB \t- store attributes\n"));
    db->maxKeyCount =  maxUniverseSize;
    db->maxSetSize = maxSetSize;
    db->maxAttrSize = maxAttrSize;

    db_createAttribute(db, "name", TYPE_STRING, DB_MAX_STRING_LENGTH);

    db->keyCount = 0;
    db->setCount = 0;
    db->attrCount = 0;

    DEBUG_CALL(printf("createEmptyDB \t- initializes sets\n"));
    db->sets = (bitset*)malloc(sizeof(bitset)*maxSetSize);
    DEBUG_CALL(printf("createEmptyDB \t- initializes attr size %ld\n", sizeof(Attributes)*maxAttrSize));
    db->attributes = malloc(sizeof(Attributes)*maxAttrSize);

    DEBUG_CALL(printf("createEmptyDB \t- done\n"));

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

    if(st_search(db->setNamesTable, name)!=NULL){
        errno = EINVAL;
        perror("db_createSet \t- set name already in use");
        return;
    }

    if(strlen(name) < 1){
        fprintf(stderr, "db_createSet \t- ERROR! %s needs to be above 0 character long\n", name);
        return;
    }
    DEBUG_CALL(printf("db_createSet \t- adding set '%s'\n", name));
    uint64_t index = db->setCount;
    DEBUG_CALL(printf("db_createSet \t- insert set at set index %ld\n", index));
    if(index >= db->maxSetSize){
        fprintf( stderr, "db_createSet \t- ERROR! Limit of amount of sets reached!\n");
        exit(0);
    }
    DEBUG_CALL(printf("\t\t- st_insert\n"));

    st_insert(db->setNamesTable, name, index);
    
    bitset_initialize(&db->sets[index], db->maxKeyCount, BIT_CLEAR);

    db->setCount++;

    DEBUG_CALL(printf("\t\t- done!\n"));
}




/**
 * @brief removes a key from a set 
 * Switches a bit to 0 at the index of the key given by the keyTable in the database
 * @param db Database to use
 * @param setKey name of the set
 * @param key name of the key to remove
 */
void db_removeFromSet(Database* db, char*setKey, char*key){
    DEBUG_CALL(printf("db_removeFromSet \t- removing %s from set %s\n", key, setKey));
    TableData* edata = st_search(db->keyTable, key);

    if(edata == NULL){
        perror("addToSet \t- ERROR! key name not in database");
        return;
    }

    TableData* sdata = st_search(db->setNamesTable, setKey);

    if(sdata == NULL){
        perror("addToSet \t- ERROR! set name not in database symbol table");
        return;
    }

    if(db->sets[sdata->index].bits == NULL){
        perror("addToSet \t- ERROR! set not in database");
        return;
    }
    //long index = edata->index % (sizeof(uint64_t)*8);
    //long index2 = edata->index / (sizeof(uint64_t)*8);

    bitset_clear_bit(&db->sets[sdata->index], edata->index);
    //db->sets[sdata->index][index2]&= ~(1 << index);
}




/**
 * @brief Adds one element to the set 
 * Switches one bit in the set to a 1 on the index corresponding to the element.
 * @param db Database to use
 * @param setKey name of the set to add element to
 * @param key name of the key
 */
void db_addToSet(Database* db, char* setKey, char* key){
    DEBUG_CALL(printf("db_addToSet \t- adding %s to set %s\n", key, setKey));
    TableData* edata = st_search(db->keyTable, key);
    if(edata == NULL){
        errno = EINVAL;
        perror("addToSet \t- ERROR! key name not in database");
        return;
    }
    TableData* sdata = st_search(db->setNamesTable, setKey);
    if(sdata == NULL){
        errno = EINVAL;
        perror("addToSet \t- ERROR! set name not in database symbol table");
        return;
    }
    if(db->sets[sdata->index].bits == NULL){
        errno = EINVAL;
        perror("addToSet \t- ERROR! set not in database");
        return;
    }
    //long index = edata->index % (sizeof(uint64_t)*8);
    //long index2 = edata->index / (sizeof(uint64_t)*8);

    bitset_set_bit(&db->sets[sdata->index], edata->index);
    //db->sets[sdata->index][index2] |= (uint64_t)1 << index;
}




/**
 * @brief Adds a new key to the database. Can't have the same name as another
 * key. Ignores dublicate keys.
 * 
 * @param db Database to add key to
 * @param name Name of the key
 */
void db_addKey(Database* db, char*name){
    DEBUG_CALL(printf("db_addKey \t- adding key '%s'\n", name));
    if(strlen(name) < 1){
        errno = EINVAL;
        perror("db_addKey \t- name of key must be longer than 0");
        return;
    }
    if(st_search(db->keyTable, name)!=NULL){
        errno = EINVAL;
        perror("db_addKey \t- key name already in use");
        return;
    }
    st_insert(db->keyTable, name, db->keyCount);
    db->keyCount++;
}





/**
 * @brief creates a new attribute table
 * If type is TYPE_UNDEFINED the data block wont be allocated.
 * Amount of elements in data depends on how many elements each attribute union can hold.
 * Name must be unique.
 * @param db database to create attribute table in
 * @param name name of the new table
 * @param type the datatype of the attributes to be stored
 * @param stringSize optional argument. Defines the maximum size of strings.
 */
void db_createAttribute(Database* db, char* name, int type, int stringSize){
    DEBUG_CALL(printf("db_createAttribute \t- name %s, type %d, with string size %d\n", name, type, stringSize));
    if(st_search(db->attrNamesTable, name)!=NULL){
        errno = EINVAL;
        perror("db_createAttribute \t- attribute name already in use");
        return;
    }

    if(db->attrCount == db->maxAttrSize){
        fprintf(stderr, "db_createAttribute \t- cannot create more attribute tables in this database\n");
        return;
    }

    if(type == TYPE_STRING && stringSize < 1){
        errno = EINVAL;
        perror("db_createAttribute \t- type is TYPE_STRING but stringSize is < 1");
        return;
    }

    size_t numberOfElements = 0;

    switch (type)
    {
    case TYPE_8:        numberOfElements = db->maxKeyCount/8;  break;
    case TYPE_STRING:   numberOfElements = db->maxKeyCount*stringSize/8;  
                        db->attributes[db->attrCount].stringLength = stringSize;
                        break;
    case TYPE_16:       numberOfElements = db->maxKeyCount/4;  break;
    case TYPE_32:       numberOfElements = db->maxKeyCount/2;  break;
    case TYPE_64:       numberOfElements = db->maxKeyCount;    break;
    default: break;
    }
    db->attributes[db->attrCount].type = type;
    db->attributes[db->attrCount].data = calloc(numberOfElements, sizeof(AttrUnion));
    st_insert(db->attrNamesTable, name, db->attrCount);
    db->attrCount++;
}



/**
 * @brief Sets the value for an entry in one attribute table.
 *  
 * @param db        database
 * @param attrName  name of the attribute table
 * @param keyName   name of the element
 * @param data      Integer or string depending on the TYPE of the attribute table. 
 *                  If string it must be smaller than the stringLength of the attribute table.
 */
void db_setAttribute(Database* db, char* attrName, char* keyName, void* data){
    DEBUG_CALL(printf("db_setAttribute \t- name %s, keyname %s\n", attrName, keyName));

    TableData* attrData = st_search(db->attrNamesTable, attrName);
    if(attrData==NULL){
        errno = EINVAL;
        perror("db_setAttribute \t- attrName not found in database");
        return;
    }
    TableData* keyData = st_search(db->keyTable, keyName);
    if(keyData==NULL){
        errno = EINVAL;
        perror("db_setAttribute \t- keyName not found in database");
        return;
    }

    long attrIndex = attrData->index;
    long keyIndex = keyData->index;
    int type = db->attributes[attrIndex].type;
    uint32_t strl = db->attributes[attrIndex].stringLength;

    
    char* text = (char*)data;
    if(type == TYPE_STRING && strlen(text)> strl){
        errno = EINVAL;
        perror("db_setAttribute \t- string data longer than max string length in attribute table");
        return;
    }

    switch (type)
    {
    case TYPE_8:        db->attributes[attrIndex].data[keyIndex/8].char_u[keyIndex%8] = *((uint8_t*) data);
                        break;

    case TYPE_STRING:   for (size_t i = 0; i < strl; i++){
                            db->attributes[attrIndex].data[(keyIndex*strl+i)/8].char_u[(keyIndex*strl+i)%8] =  text[0];
                            text++;
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
 */
void db_destroy(Database* db){
    DEBUG_CALL(printf("db_destroy \t- begin\n")) 
    //  free sets
    for (size_t i = 0; i < db->setCount; i++){
        free(db->sets[i].bits);
    }
    free(db->sets);

    //  free attributes
    for (size_t i = 0; i < db->attrCount; i++)
    {
        free(db->attributes[i].data);
    }
    free(db->attributes);

    //  free symbol tables
    free_table(db->keyTable);
    free_table(db->setNamesTable);
    free_table(db->attrNamesTable);

    free(db);
    DEBUG_CALL(printf("db_destroy \t- success\n")) 
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

        bitset* s = &db->sets[index];
        //uint64_t* set = db->sets[index];

        printf("set %s", keys[i]);
        
        //long b = db->keyCount / (sizeof(uint64_t)*8)+1;

        bitset_print(s);
        /*
        for (long j = 0; j < b; j++){
            printf("\n");
            uint64_t k = set[j];
            printBits(sizeof(uint64_t),k);
        }
        */
        printf("\n");
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
        uint32_t index = data->index;
        Attributes attr = db->attributes[index];
        uint32_t strl = attr.stringLength;


        printf("\n-Attribute %s-\n[", keys[i]);

        for (uint64_t keyIndex = 0; keyIndex < db->keyCount; keyIndex++){

            if(attr.type == TYPE_UNDEFINED){
                printf("undefined");
                break;
            }
            switch (attr.type)
            {
            case TYPE_8: printf("%u", attr.data[keyIndex/8].char_u[keyIndex%8]);break;
            case TYPE_STRING:   
                for (size_t k = 0; k < strl; k++){
                    char c = attr.data[(keyIndex*strl+k)/8].char_u[(keyIndex*strl+k)%8];
                    if(c != '\0') {
                        printf("%c",c);
                    } else break;
                }
                break;
            printf("%lu", attr.data[keyIndex].longlong_u);break;
            case TYPE_16: printf("%u", attr.data[keyIndex/4].int_u[keyIndex%4]);break;
            case TYPE_32: printf("%u", attr.data[keyIndex/2].long_u[keyIndex%2]);break;
            case TYPE_64: printf("%lu", attr.data[keyIndex].longlong_u);break;
            default:break;
            }
            printf(",");
            if(keyIndex%16==0 && keyIndex != 0){
                printf("\n");
            }
        }
        printf("]\n");
        i++;
    }
}









void db_test(void){
	Database* db = createEmptyDB(DB_MAX_STRING_LENGTH, 10, 10); 

	char s[] = "coolsetname";
	db_createSet(db, s);

	// add keys
	for (int i = 0; i < 150; i++){
		char dst[12];
		sprintf(dst, "key%d", i);
		db_addKey(db, dst);
        if(i%2){
            db_addToSet(db, s, dst);
        }
	}
	
	char key[] = "key5";

	db_addToSet(db, s, key);

	char attr1[] = "attributetable_byte";
	db_createAttribute(db, attr1, TYPE_8, -1);
	db_createAttribute(db, attr1, TYPE_16, -1);
	char attr2[] = "attributetable_int";
	db_createAttribute(db, attr2, TYPE_16, -1);
	char attr3[] = "attributetable_long";
	db_createAttribute(db, attr3, TYPE_32, -1);
	char attr4[] = "attributetable_longlong";
	db_createAttribute(db, attr4, TYPE_64, -1);
	char attr5[] = "attributetable_undefined";
	db_createAttribute(db, attr5, TYPE_UNDEFINED, -1);
	char attr6[] = "attributetable_string";
	db_createAttribute(db, attr6, TYPE_STRING, 20);


	uint64_t data = CHAR_MAX/2;
	db_setAttribute(db, attr1, key, &data);
	
	data = INT_MAX/2;
	db_setAttribute(db, attr2, key, &data);

	data = LONG_MAX/2;
	db_setAttribute(db, attr3, key, &data);

	data = LLONG_MAX/2;
	db_setAttribute(db, attr4, key, &data);

	data = LLONG_MAX/2;
	db_setAttribute(db, attr5, key, &data);


    char datas[] = "hej jag heter karl";
	db_setAttribute(db, attr6, key, datas);

    char datas2[] = "AAAABBBBEEEERRRRTTTTTT";
    char key2[] = "key4";
	db_setAttribute(db, attr6, key2, datas2);


	db_print(db);

	db_destroy(db);
}
