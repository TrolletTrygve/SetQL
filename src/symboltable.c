#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datastructures/symboltable.h"
 
#define CAPACITY 50000 // Size of the Hash Table

static hash_t digest(char* ssn, uint32_t len) {
    uint32_t hash = 5381;
    for(uint32_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + (uint32_t)ssn[i];
    }
    return (hash_t) (hash % 256);
}

hash_t hash_name(char* ssn) {
    return digest(ssn, 12);
}



TableData* create_TableData(char* name, uint64_t index) {
    // Creates a pointer to a new hash table TableData
    TableData* tableData =  (malloc (sizeof (struct TableData)));
    tableData->name = (char*) malloc (strlen(name) + 1);

    strcpy(tableData->name, name);
    tableData->index = index;

    return tableData;
}
 



SymbolTable* create_table(int size) {
    // Creates a new SymbolTable
    SymbolTable* table = (SymbolTable*) malloc (sizeof(SymbolTable));
    table->size = size;
    table->count = 0;
    table->TableData =  (TableData**) calloc (table->size, sizeof (TableData*));
    for (int i=0; i<table->size; i++)
        table->TableData[i] = NULL;
    return table;
}



void free_TableData(SymbolTable* table, TableData* tableData){
    // Frees an TableData
    free(tableData->name);
    free(tableData);
    table->count--;
}
 

 
void free_table(SymbolTable* table) {
    // Frees the table
    for (int i = 0; i < table->size; i++)
    {
        TableData* tb = table->TableData[i];
        if(tb != NULL){
            free_TableData(table, tb);
        }
    }
    free(table->TableData);
    free(table);
}

void st_insert(SymbolTable* table, char* name, uint64_t index) {
    // Create the TableData
    TableData* tableData = create_TableData(name, index);

    // Compute the index
    TableData* current_tableData;
    for(int i = 0; i < table->size; i++){

        current_tableData = table->TableData[i];
        if (current_tableData == NULL) {
            if (table->count == table->size) {
                // Hash Table Full
                printf("Insert Error: Hash Table is full\n");
                // Remove the create TableData
                free_TableData(table, tableData);
                return;
            }
            table->TableData[i] = tableData; 
            table->count++;
            return;
        // Insert directly
        }
        else if(strcmp(tableData->name, table->TableData[i]->name) == 0){
            printf("st_insert \t- ERROR");
            exit(1);
        }
    }
}
 
void* st_search(SymbolTable* table, char* name) {
    // Searches the name in the hashtable
    // and returns NULL if it doesn't exist
    TableData* tableData;
    for(int i = 0; i < table->size; i++){
        tableData = table->TableData[i];
        if (tableData != NULL) {
            if (strcmp(tableData->name, name) == 0){
                return tableData;
            }
        }
    }
    return NULL;
}

void print_search(SymbolTable* table, char* ssn) {
    char* val;
    if ((val = st_search(table, ssn)) == NULL) {
        printf("ssn:%s does not exist\n", ssn);
        return;
    }
    else {
        printf("ssn:%s, name:%s\n", ssn, val);
    }
}
 
void print_table(SymbolTable* table) {
    for (int i=0; i<table->size; i++) {
        if (table->TableData[i]) {
            printf("TableIndex:%d, index: %ld name:%s\n", i, table->TableData[i]->index, table->TableData[i]->name);
        }
    }
}


/*
    function that returns a set of keys from the table.
    FLAG        -   if 1: return all keys. if 0: return upper keys
    return array of ssn key strings
*/
char** st_getKeys(SymbolTable* table){
    //allocate memory for keys
    char** keys = malloc(table->size*sizeof(char*));
    //copy all keys to array
    int j = 0;
    for (int i = 0; i < table->size; i++)
    {
        TableData* tableData = table->TableData[i];
        if(tableData != NULL){
            keys[j] = strdup(tableData->name);
            j++;
        }
    }
    return keys;
}

int getCount(SymbolTable* table){
    return table->count;
}