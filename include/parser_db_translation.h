#ifndef __PARSER_DB_TRANSLATION__
#define __DBMS_NETWORKING_H__

#include"datastructures/database.h"
#include "parser.h"
#include"utils.h"
#include"datastructures/bitset.h"

typedef struct ReturnBitset {
    bitset* bs;
    int free;
} ReturnBitset;

void db_addParsedData(Database* db, universe* u);
ReturnBitset db_parse_set_operation(Database* db, set_op* sop);


#endif