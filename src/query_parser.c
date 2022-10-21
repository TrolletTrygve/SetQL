#include "parser.h"

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <ctype.h>
#include <assert.h>
#include <regex.h>
//#include <stdarg.h>

// SUPPORT FUNCTIONS 

static char* str_copy(const char* str) {
    char* new_str = (char*) malloc((strlen(str) + 1) * sizeof(char));
    strcpy(new_str, str);
    return new_str;
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