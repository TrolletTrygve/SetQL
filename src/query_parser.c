#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <ctype.h>
#include <assert.h>
#include <regex.h>
//#include <stdarg.h>

// static const char* OP_COMPLEMENT_STRING = "COMPLEMENT";
// static const char* OP_UNION_STRING = "UNION";
// static const char* OP_INTERSECTION_STRING = "INTERSECTION";
// static const char* OP_DIFFERENCE_STRING = "DIFFERENCE";

// SUPPORT FUNCTIONS 

static char* str_copy(const char* str) {
    char* new_str = (char*) malloc((strlen(str) + 1) * sizeof(char));
    strcpy(new_str, str);
    return new_str;
}

// It copies a string from start_index to end_index-1 (It does a malloc).
static char* str_copy_idx(const char* str, size_t start_index, size_t end_index) {
    size_t len = strlen(str);
    assert(start_index <= len && end_index <= len);
    char strCopy[len + 1];
    strcpy(strCopy, str);
    strCopy[end_index] = '\0';
    char* new_str = strCopy + start_index;
    return str_copy(new_str);
}

set_op* create_empty_set_op(void) {
    set_op* op = (set_op*)malloc(sizeof(set_op));
    op->is_leave = 0;
    op->set_name = NULL;
    op->left_op = NULL;
    op->right_op = NULL;
    op->op_type = 0;
    return op;
}

void free_set_op(set_op* op) {
    assert(op != NULL);

    if (op->set_name != NULL) {
        free(op->set_name);
        op->set_name = NULL;
    }
    if (op->left_op != NULL) {
        free_set_op(op->left_op);
        op->left_op = NULL;
    }
    if (op->right_op != NULL) {
        free_set_op(op->right_op);
        op->right_op = NULL;
    }
    op->is_leave = 0;
    op->op_type = 0;
}

static set_op* create_set_op_complement(set_op* left_op, set_op* right_op) {
    assert(right_op == NULL);
    set_op* op = create_empty_set_op();
    op->is_leave = 0;   // is_leave = FALSE
    op->op_type = OP_COMPLEMENT;
    op->left_op = left_op;
    return op;
}

static set_op* create_set_op_union(set_op* left_op, set_op* right_op) {
    set_op* op = create_empty_set_op();
    op->is_leave = 0;   // is_leave = FALSE
    op->op_type = OP_UNION;
    op->left_op = left_op;
    op->right_op = right_op;
    return op;
}

static set_op* create_set_op_intersection(set_op* left_op, set_op* right_op) {
    set_op* op = create_empty_set_op();
    op->is_leave = 0;   // is_leave = FALSE
    op->op_type = OP_INTERSECTION;
    op->left_op = left_op;
    op->right_op = right_op;
    return op;
}

static set_op* create_set_op_difference(set_op* left_op, set_op* right_op) {
    return create_set_op_intersection(left_op, create_set_op_complement(right_op, NULL));
}

set_op* create_set_op(set_op* left_op, set_op* right_op, int op_type) {
    if (op_type == OP_COMPLEMENT)
        return create_set_op_complement(left_op, right_op);
    if (op_type == OP_UNION)
        return create_set_op_union(left_op, right_op);
    if (op_type == OP_INTERSECTION)
        return create_set_op_intersection(left_op, right_op);
    if (op_type == OP_DIFFERENCE)
        return create_set_op_difference(left_op, right_op);
    return NULL;
}

set_op* create_set_op_leave(const char* set_name) {
    set_op* op = create_empty_set_op();
    op->is_leave = 1;   // is_leave = TRUE
    op->set_name = str_copy(set_name);
    return op;
}

query create_empty_query(void) {
    query q;
    initialize_string_list(&q.column_names, 0);
    q.op = NULL;
    return q;
}

void free_query(query* q) {
    assert(q != NULL);
    free_string_list(&q->column_names);
    if (q->op != NULL) {
        free_set_op(q->op);
        q->op = NULL;
    }
}

// REGEX STAFF

static const char* regex_string_query = "^\\s*SELECT\\s+([a-zA-Z0-9_]+\\s*(,\\s*[a-zA-Z0-9_]+\\s*)*)\\s+FROM\\s+([^;]+);$";
static regex_t regex_query;

// Returns 0 if succesful
static int parse_entire_query(query* q, const char* query_string, char* error_message) {
    assert(q != NULL);
    size_t maxGroups = 4;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    const char* cursor = query_string;

    if (regexec(&regex_query, cursor, maxGroups, group_array, 0)) {
        strcpy(error_message, "Error parsing the query. Does not match the pattern: SELECT <name_1>, ... FROM <set_operation> ;");
        return 1;
    }

    for (size_t g = 0; g < maxGroups; g++)
    {
        groups[g] = NULL;
        if (group_array[g].rm_eo != -1)
            groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc

    }

    char* column_names = groups[1];
    char* set_operation = groups[3];

    printf("column_names=%s\n", column_names);  // TOREMOVE
    printf("set_operation=%s\n", set_operation);  // TOREMOVE

    for (size_t g = 0; g < maxGroups; g++) {
        if (groups[g] != NULL)
            free(groups[g]);
    }

    return 0;
}

static int regex_is_initialized = 0;

// Compile all the regex expressions that will be used in the parsing
static void initialize_regex(void) {
    if (regex_is_initialized) return;
    
    assert(regcomp(&regex_query, regex_string_query, REG_EXTENDED | REG_ICASE) == 0);

    regex_is_initialized = 1;
}

// If succesful returns 0 and modifies the query
int parse_query(query* q, const char* query_string) {
    initialize_regex();
    char error_message[200];
    if (parse_entire_query(q, query_string, error_message)) {
        printf("%s\n", error_message);
    }
    // TODO

    return -1;
}