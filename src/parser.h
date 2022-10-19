#ifndef PARSER_H_
#define PARSER_H_

#include <stddef.h>

#define NO_TYPE -1
#define STRING_TYPE 1
#define INTEGER_TYPE 2
#define FLOAT_TYPE 3

int type_name2data_type(const char* type_name);

const char* data_type2type_name(int data_type);

typedef struct {
    char **strings;
    size_t length;
} string_list;

typedef struct {
    char*** values;                 // E.g. [["crow", 100000], ["chicken", 25000000000], ["bat", 3000000000]]
    size_t length;                  // E.g. 3
} array_list;

typedef struct {
    char name[50];
    array_list key_values;
} set;

typedef struct {
    char name[50];
    string_list key_data_type_names;        // E.g. {['STRING'], 1}
    string_list key_data_names;             // E.g. {['scientific_name'], 1}
    string_list attribute_data_type_names;  // E.g. {['STRING', 'INTEGER'], 2}
    string_list attribute_data_names;       // E.g. {['name', 'population'], 2}
    array_list key_values;                  // E.g. [["Gallus gallus domesticus"], ["Chiroptera"], ["Corvus"]]
    array_list attribute_values;            // E.g. [["crow", 100000], ["chicken", 25000000000], ["bat", 3000000000]]
    set *sets;                              // E.g. [Set{name: "Bird", key_values: [["Gallus gallus domesticus"], ["Corvus"]}, Set{name: "CanFly", key_values: [["Corvus"]}]
    size_t sets_length;                     // E.g. 2
} universe;

universe create_universe_example(void);

void print_universe(universe u);

void print_string_list(string_list strings);

// Note: This parser will not work with the chars ';' and ':' inside of strings
int parse_initialization(universe* u, const char* file_name);    // TODO

#endif