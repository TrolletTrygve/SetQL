#include "parser_db_translation.h"

/**
 * @brief helper functions for db_addParsedData
 * 
 * @param db database
 * @param u universe data to add to database
 * @return 1 if error, 0 if no error
 */
int db_addParsedData_keys(Database* db, universe* u);
int db_addParsedData_sets(Database* db, universe* u);
int db_addParsedData_attributes(Database* db, universe* u);
void freeBitsets(SetOpReturn rbl, SetOpReturn rbr);


int db_addParsedData(Database* db, universe* u){
    char* uname = u->key_data_names.strings[0];
    if(db->primaryKeyName[0] == '\0'){
        strcpy(db->primaryKeyName, uname);
        db_createAttribute(db, uname, TYPE_STRING, DB_MAX_STRING_LENGTH);
    }
    else if(strlen(db->primaryKeyName)> 0 && strcmp(db->primaryKeyName, uname) != 0){
        DEBUG_CALL(perror("db_addParsedData \t- universe name different from database primary key name"));
        return 1;
    }
    if(db_addParsedData_keys(db, u)){
        return 1;
    }
    if(db_addParsedData_sets(db, u)){
        return 1;
    }
    if(db_addParsedData_attributes(db, u)){
        return 1;
    }
    return 0;
}


/**
 * @brief adds the first key of the key values in the universe to the keytable of the database.
 * 
 * @param db database
 * @param u universe
 * TODO: handle multiple keys (change symbol table to handle an array of key strings and add the hash together)
 */
int db_addParsedData_keys(Database* db, universe* u){
    if(u->key_values.length > db->maxKeyCount - db->keyCount){
        fprintf(stderr, "%ld, %ld, %ld", u->key_values.length,db->maxKeyCount,db->keyCount);
        fprintf(stderr, "db_addParsedData_keys \t- error, more keys added than database can handle");
        return 1;
    }
    for (uint64_t i = 0; i < u->key_values.length; i++){
        db_addKey(db, u->key_values.values[i][0]);
        db_setAttribute(db, db->primaryKeyName, u->key_values.values[i][0], u->key_values.values[i][0]);
        //st_insert(db->keyTable, u->key_values.values[i][0],db->keyCount);
        //db->keyCount++;
    }
    return 0;
}


int db_addParsedData_sets(Database* db, universe* u){
    if(u->sets_length > db->maxSetSize-db->setCount){
        fprintf(stderr, "db_addParsedData_sets \t- error, more sets added than database can handle");
        return 1;
    }
    for (uint64_t i = 0; i < u->sets_length; i++){
        db_createSet(db, u->sets[i].name);
        for (uint64_t j = 0; j < u->sets[i].key_values.length; j++){
            db_addToSet(db, u->sets[i].name, u->sets[i].key_values.values[j][0]);
        }
    }
    return 0;
}


/**
 * TODO: Handle different integer sizes here
 * TODO: Handle differen string sizes
 */
int db_addParsedData_attributes(Database* db, universe* u){
    if(u->attribute_data_names.length > db->maxAttrSize-db->attrCount){
        fprintf(stderr, "db_addParsedData_attributes \t- error, more attributes added than database can handle");
        return 1;
    }
    for (uint64_t i = 0; i < u->attribute_data_names.length; i++){
        char* uname = u->attribute_data_names.strings[i];
        char* utype = u->attribute_data_type_names.strings[i];
        if(!strcmp(utype,"STRING")){
            db_createAttribute(db, uname, TYPE_STRING, DB_MAX_STRING_LENGTH);
        }
        else if(!strcmp(utype,"INTEGER")){
            db_createAttribute(db, uname, TYPE_64, 0);
        }
        else{
            db_createAttribute(db, uname, TYPE_UNDEFINED, 0);
            fprintf(stderr, "db_addParsedData_attributes \t- type is not INTEGER or STRING");
            return 1;
        }
        if(!strcmp(utype,"STRING")){
            for (uint64_t k = 0; k < u->key_values.length; k++){
                // maybe works idk???
                db_setAttribute(db, uname, u->key_values.values[k][0], u->attribute_values.values[k][i]);
            }
        }
        else if(!strcmp(utype,"INTEGER")){
            for (uint64_t k = 0; k < u->key_values.length; k++){
                // maybe works idk???
                char *eptr;
                long l = strtol(u->attribute_values.values[k][i], &eptr, 10);
                db_setAttribute(db, uname, u->key_values.values[k][0], (void*)&l);
            }
        }
        else{
            db->attributes[i].type = TYPE_UNDEFINED;
            fprintf(stderr, "db_addParsedData_attributes \t- type is not INTEGER or STRING");
            return 1;
        }
        
    }
    return 0;
}
/**
 * @brief helper function for db_parse_set_operation
 * 
 * @param rbl bitset to maybe clear
 * @param rbr bitset to maybe clear
 */
void freeBitsets(SetOpReturn rbl, SetOpReturn rbr){
    if(rbl.free){
        DEBUG_CALL(printf("db_run_set_operation \t- free(rbl.bs)\n"));
        free(rbl.bs);
        }
    if(rbr.free){
        DEBUG_CALL(printf("db_run_set_operation \t- free(rbr.bs)\n"));
        free(rbr.bs);
        }
}


/**
 * @brief performs the set operation queries
 * 
 * 
 * @param db database
 * @param sop set operation structure (querie)
 * @return ReturnBitset, use field .bs for resulting bitset. .bs is allocated memory.
 * 
 */
SetOpReturn db_run_set_operation(Database* db, set_op* sop){
    DEBUG_CALL(printf("db_run_set_operation \t- running set operation, optype %d, is_leave %d\n",sop->op_type, sop->is_leave ));

    SetOpReturn rbl;
    SetOpReturn rbr;
    SetOpReturn rbreturn;
    rbreturn.free = 1;

    bitset* result;

    if(sop->is_leave){
        TableData* tb = st_search(db->setNamesTable, sop->set_name);
        SetOpReturn rb = {.bs = &db->sets[tb->index], .free = 0};
        return rb;
    }
    else{
        switch (sop->op_type){
        case OP_COMPLEMENT:
                rbl = db_run_set_operation(db, sop->left_op);
                result = bitset_complement(rbl.bs);
                if(rbl.free){free(rbl.bs);}
                rbreturn.bs = result;
                return rbreturn;      
            break;
        case OP_UNION:
                rbl = db_run_set_operation(db, sop->left_op);
                rbr = db_run_set_operation(db, sop->right_op);
                result = bitset_union(rbl.bs, rbr.bs);
                freeBitsets(rbl, rbr);
                rbreturn.bs = result;
                return rbreturn;
            break;
        case OP_INTERSECTION:
                rbl = db_run_set_operation(db, sop->left_op);
                rbr = db_run_set_operation(db, sop->right_op);
                result = bitset_intersection(rbl.bs, rbr.bs);
                freeBitsets(rbl, rbr);
                rbreturn.bs = result;
                return rbreturn;
            break;
        case OP_DIFFERENCE:
                rbl = db_run_set_operation(db, sop->left_op);
                rbr = db_run_set_operation(db, sop->right_op);
                result = bitset_difference(rbl.bs, rbr.bs);
                freeBitsets(rbl, rbr);
                rbreturn.bs = result;
                return rbreturn;
            break;
        default:
            fprintf(stderr, "db_parse_set_operation \t- ERROR something went wrong. op_type flag not [1-4].\n");
            break;
        }
    }
    return rbreturn;
}



QueryReturn* db_run_query(Database* db, query* q){
    DEBUG_CALL(printf("db_run_query \t- running query\n"));
    QueryReturn* qr = malloc(sizeof(QueryReturn));
    qr->columns     = calloc(q->column_names.length, sizeof(ColumnData));
    qr->dataLength  = 0;
    qr->columnCount = q->column_names.length;
    SetOpReturn sor = db_run_set_operation(db, q->op);
    bitset_print(sor.bs);
    int attributeTables[db->attrCount];
    memset(attributeTables,0,sizeof(int)*db->attrCount);

    DEBUG_CALL(printf("db_run_query \t- getting attributes\n"));
    // get all attribute tables, initialize ColumnData structures
    for (size_t column_index = 0;column_index < q->column_names.length; column_index++){
        ColumnData cd;
        TableData* tb = st_search(db->attrNamesTable, q->column_names.strings[column_index]);
        attributeTables[column_index] = tb->index;
        // if attribute type is string
        if(db->attributes[tb->index].type == TYPE_STRING){
            cd.isString = 1;
            cd.data = malloc(db->keyCount*DB_MAX_STRING_LENGTH*sizeof(uint8_t));
        }
        else{
            cd.isString = 0;
            cd.data = calloc(db->keyCount, sizeof(uint64_t));
        }        
        qr->columns[column_index] = cd;
    }

    DEBUG_CALL(printf("db_run_query \t- adding attribute values to return data structure\n"));
    // add all attributes of keys in the set operation return to the query return
    for (uint64_t int_index = 0; int_index < sor.bs->integer_count; int_index++){
        uint64_t ll = sor.bs->bits[int_index];
        for (int byte_index = 0; byte_index < INTEGER_BIT_SIZE; byte_index++){
            uint64_t bit = (ll >> byte_index);
            if(bit & 1){
                for (size_t col_i = 0; col_i < q->column_names.length; col_i++){
                    Attributes attr = db->attributes[attributeTables[col_i]];
                    if(attr.type == TYPE_STRING){
                        uint8_t* coldata        = (uint8_t*) qr->columns[col_i].data;
                        uint64_t dest_address   = qr->dataLength*DB_MAX_STRING_LENGTH;
                        uint64_t src_address    = (int_index*INTEGER_BIT_SIZE+byte_index)*DB_MAX_STRING_LENGTH;
                        uint8_t* data           = (uint8_t*) attr.data;
                        memcpy(&coldata[dest_address], &data[src_address], DB_MAX_STRING_LENGTH);
                        qr->columns[col_i].memorySize += DB_MAX_STRING_LENGTH;
                    }
                    else{ 
                        uint64_t* coldata = (uint64_t*) qr->columns[col_i].data;
                        coldata[qr->dataLength] = attr.data[int_index*INTEGER_BIT_SIZE+byte_index].int_64_u;
                        qr->columns[col_i].memorySize += sizeof(uint64_t);
                    }
                }
                qr->dataLength++;
            }
        }
    }
    // free data here
    DEBUG_CALL(printf("db_run_query \t- done\n"));
    return qr;
}
