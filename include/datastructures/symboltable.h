#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define hash_t uint8_t


// Defination of TableDatas
typedef struct TableData {
    uint64_t index;
	char* name;
}TableData;
 
// Defination of hash table
typedef struct SymbolTable {
    TableData** TableData;
    int size;
    int count;
}SymbolTable;


//FUNCTIONS
hash_t hash_name(char* name);
TableData* create_TableData(char* name, uint64_t index);
SymbolTable* create_table(int size);
void free_TableData(SymbolTable* table, TableData* tableData);
void free_table(SymbolTable* table);
void handle_collision(SymbolTable* table, unsigned long index, TableData* tableData);
void st_insert(SymbolTable* table, char* name, uint64_t index);
void* st_search(SymbolTable* table, char* name);
void print_search(SymbolTable* table, char* name);
void print_table(SymbolTable* table);
char** st_getKeys(SymbolTable* table);
int getCount(SymbolTable* table);

#endif 