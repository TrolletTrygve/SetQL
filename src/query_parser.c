#include "parser.h"

//#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
#include <assert.h>
#include <regex.h>
//#include <stdarg.h>


set_op* create_empty_set_op(void) {
    set_op* op = (set_op*)malloc(sizeof(set_op));
    op->is_leave = 0;
    op->set_name = NULL;
    op->left_op = NULL;
    op->right_op = NULL;
    op->op_value = 0;
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
    op->op_value = 0;
}