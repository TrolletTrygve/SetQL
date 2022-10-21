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

void initialize_string_list(string_list* sl_ptr, size_t length);
void free_string_list(string_list* sl_ptr);

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

universe create_empty_universe(void);

void free_universe(universe* u);

universe create_universe_example(void);

void print_universe(universe u);

void print_string_list(string_list strings);

// Note: This parser will not work with the char ';' inside of strings
int parse_initialization(universe* u, const char* file_name);    // TODO

// QUERY DEFINITIONS AND STRUCTS

#define OP_COMPLEMENT   1
#define OP_UNION        2
#define OP_INTERSECTION 3
#define OP_DIFFERENCE   4   // This type will not appear in the struct set_operation (INTERSECTION and COMPLEMENT will be used instead)

struct set_operation {
    int is_leave;
    char* set_name; // NULL if ! is_leave
    // If it is not a leave node
    struct set_operation* left_op;
    struct set_operation* right_op;
    int op_type;
};

typedef struct set_operation set_op;

set_op* create_empty_set_op(void);

void free_set_op(set_op* op);

set_op* create_set_op_leave(const char* set_name);
set_op* create_set_op(set_op* left_op, set_op* right_op, int op_type);

typedef struct {
    string_list column_names;
    set_op* op;
} query;

query create_empty_query(void);

void free_query(query* q);

#endif