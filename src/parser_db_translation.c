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
void freeBitsets(ReturnBitset rbl, ReturnBitset rbr);


void db_addParsedData(Database* db, universe* u){
    if(db_addParsedData_keys(db, u)){
        return;
    }
    if(db_addParsedData_sets(db, u)){
        return;
    }
    if(db_addParsedData_attributes(db, u)){
        return;
    }
}


/**
 * @brief adds the first key of the key values in the universe to the keytable of the database.
 * 
 * @param db database
 * @param u universe
 * TODO: handle multiple keys (change symbol table to handle an array of key strings and add the hash together)
 */
int db_addParsedData_keys(Database* db, universe* u){
    if(u->key_values.length> db->maxKeyCount - db->keyCount){
        fprintf(stderr, "db_addParsedData_keys \t- error, more keys added than database can handle");
        return 1;
    }
    for (uint64_t i = 0; i < u->key_values.length; i++){
        st_insert(db->keyTable, u->key_values.values[i][0],db->keyCount);
        db->keyCount++;
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
        for (uint64_t j = 0; j < u->sets->key_values.length; j++){
            db_addToSet(db, u->sets[i].name, u->sets->key_values.values[j][0]);
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
            db_createAttribute(db, uname, TYPE_STRING, 200);
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
void freeBitsets(ReturnBitset rbl, ReturnBitset rbr){
    if(rbl.free){free(rbl.bs);}
    if(rbr.free){free(rbr.bs);}
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
ReturnBitset db_parse_set_operation(Database* db, set_op* sop){
    ReturnBitset rbl;
    ReturnBitset rbr;
    ReturnBitset rbreturn;
    rbreturn.free = 1;

    bitset* result;

    if(sop->is_leave){
        TableData* tb = st_search(db->setNamesTable, sop->set_name);
        ReturnBitset rb = {.bs = &db->sets[tb->index], .free = 0};
        return rb;
    }
    else{
        switch (sop->op_type){
        case OP_COMPLEMENT:
                rbl = db_parse_set_operation(db, sop->left_op);
                result = bitset_complement(rbl.bs);
                if(rbl.free){free(rbl.bs);}
                rbreturn.bs = result;
                return rbreturn;      
            break;
        case OP_UNION:
                rbl = db_parse_set_operation(db, sop->left_op);
                rbr = db_parse_set_operation(db, sop->right_op);
                result = bitset_union(rbl.bs, rbr.bs);
                freeBitsets(rbl, rbr);
                rbreturn.bs = result;
                return rbreturn;
            break;
        case OP_INTERSECTION:
                rbl = db_parse_set_operation(db, sop->left_op);
                rbr = db_parse_set_operation(db, sop->right_op);
                result = bitset_intersection(rbl.bs, rbr.bs);
                freeBitsets(rbl, rbr);
                rbreturn.bs = result;
                return rbreturn;
            break;
        case OP_DIFFERENCE:
                rbl = db_parse_set_operation(db, sop->left_op);
                rbr = db_parse_set_operation(db, sop->right_op);
                result = bitset_intersection(rbl.bs, rbr.bs);
                freeBitsets(rbl, rbr);
                rbreturn.bs = result;
                return rbreturn;
            break;
        default:
            fprintf(stderr, "db_parse_set_operation \t- ERROR something went wrong. op_type flag not [1-4].");
            break;
        }
    }
    return rbreturn;
}