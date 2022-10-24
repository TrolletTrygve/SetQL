#ifndef __PARSER_DB_TRANSLATION__
#define __DBMS_NETWORKING_H__

#include"datastructures/database.h"
#include "parser.h"
#include"utils.h"
#include"datastructures/bitset.h"

typedef struct {
    bitset* bs;
    int free;
} SetOpReturn;

typedef struct{
    int isString; // otherwise 64bit int
    void* data;
    uint64_t memorySize;
}ColumnData;

typedef struct{
    uint64_t dataLength;
    ColumnData* data; // include keys
    uint64_t columnCount;
} QueryReturn;

void db_addParsedData(Database* db, universe* u);
SetOpReturn db_run_set_operation(Database* db, set_op* sop);

QueryReturn* db_run_query(Database* db, query* q);

#endif