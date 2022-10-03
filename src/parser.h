#ifndef PARSER_H_
#define PARSER_H_

#include <stddef.h>

//const char* DATA_TYPE_NAMES[] = {"STRING", "INTEGER"};
//const size_t DATA_TYPE_NAMES_LENGTH = 2;

typedef struct {
    char **strings;
    size_t length;
} string_list;

typedef struct {
    string_list data_type_names;
    void *values;
    size_t length;
} array_list;

typedef struct {
    char name[50];
    array_list key_values;
} set;

typedef struct {
    char name[50];
    set *sets;
    array_list key_values;
    array_list attribute_values;
} universe;

universe create_universe_example(void); // TODO

void print_universe(universe u);    // TODO

#endif