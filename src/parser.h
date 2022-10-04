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
    char*** values;                 // E.g. [['crow', 'chicken', 'bat'], [100000, 25000000000, 3000000000]]
    size_t length;                  // E.g. 3
} array_list;

typedef struct {
    char name[50];
    array_list key_values;
} set;

typedef struct {
    char name[50];
    set *sets;
    string_list key_data_type_names;        // E.g. {['STRING'], 1}
    string_list key_data_names;             // E.g. {['scientific_name'], 1}
    string_list attribute_data_type_names;  // E.g. {['STRING', 'INTEGER'], 2}
    string_list attribute_data_names;       // E.g. {['name', 'population'], 2}
    array_list key_values;
    array_list attribute_values;
} universe;

universe create_universe_example(void); // TODO

void print_universe(universe u);    // TODO

void print_string_list(string_list strings);

#endif