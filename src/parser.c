/**
 * @file parser.c
 * @author Jose Ruiz Alarcon
 * @brief This file contains implementations for the header file "parser.h".
 * It implements different functions for the type universe, set, array_list and string_list.
 * @version 0.1
 * @date 2022-10-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <regex.h>
#include <stdarg.h>

#define MAX_ULONG ((size_t) -1)
#define TRUE 1
#define FALSE 0

// SUPPORT FUNCTIONS 

static char* str_copy(char* str) {
    char* new_str = (char*) malloc((strlen(str) + 1) * sizeof(char));
    strcpy(new_str, str);
    return new_str;
}

// It copies a string from start_index to end_index-1 (It does a malloc).
static char* str_copy_idx(char* str, size_t start_index, size_t end_index) {
    size_t len = strlen(str);
    assert(start_index <= len && end_index <= len);
    char strCopy[len + 1];
    strcpy(strCopy, str);
    strCopy[end_index] = '\0';
    char* new_str = strCopy + start_index;
    return str_copy(new_str);
}

// It modifies the string to uppercase
static void str2upper(char* str) {
    for (size_t i = 0; str[i]!='\0'; i++) {
        str[i] = toupper(str[i]);
    }
}

// Concat n strings and return the result doing a malloc
static char* str_concat(int n, ...)
{
    size_t length = 0;
    // Declaring pointer to the
    // argument list
    va_list ptr;
    // Initializing argument to the
    // list pointer
    va_start(ptr, n);
    for (int i = 0; i < n; i++) {
        // Accessing current variable
        // and pointing to next one
        const char* str = va_arg(ptr, const char*);
        length += strlen(str);
    }
    
    char* new_str = (char*) malloc((length + 1) * sizeof(char));
    strcpy(new_str, "");
    // Initializing argument to the
    // list pointer
    va_start(ptr, n);
    for (int i = 0; i < n; i++) {
        // Accessing current variable
        // and pointing to next one
        const char* str = va_arg(ptr, const char*);
        strcat(new_str, str);
    }
 
    // Ending argument list traversal
    va_end(ptr);
 
    return new_str;
}

static char** copy_string_array(const char** strings, size_t length) {
    char** copy = (char**)malloc(length * sizeof(char*));
    for (size_t i = 0; i < length; i++) {
        copy[i] = (char*)malloc((strlen(strings[i])+1) * sizeof(char));
        strcpy(copy[i], strings[i]);
    }
    return copy;
}

static void modify_string_list(string_list* str_list, const char** strings, size_t length) {
    char** copy = copy_string_array(strings, length);
    str_list->length = length;
    str_list->strings = copy;
}

static char** copy_string_array2(char** strings, size_t length) {
    char** copy = (char**)malloc(length * sizeof(char*));
    for (size_t i = 0; i < length; i++) {
        copy[i] = str_copy(strings[i]);
    }
    return copy;
}

static void modify_string_list2(string_list* str_list, char** strings, size_t length) {
    char** copy = copy_string_array2(strings, length);
    str_list->length = length;
    str_list->strings = copy;
}

static char*** copy_string_matrix2d(const char** values[], size_t length_1, size_t lenght_2) {
    char*** copy = (char***)malloc(length_1 * sizeof(char**));
    for(size_t i = 0; i < length_1; i++) {
        copy[i] = copy_string_array(values[i], lenght_2);
    }
    return copy;
}

void initialize_string_list(string_list* sl_ptr, size_t length) {
    sl_ptr->length = length;
    if (length > 0) {
        sl_ptr->strings = (char**)malloc(length * sizeof(char*));
        for(size_t i = 0; i < length; i++)
            sl_ptr->strings[i] = NULL;
    } else {
        sl_ptr->strings = NULL;
    }
}

static void initialize_array_list(array_list* al_ptr, size_t length) {
    al_ptr->length = length;
    if (length > 0) {
        al_ptr->values = (char***)malloc(length * sizeof(char**));
        for(size_t i = 0; i < length; i++)
            al_ptr->values[i] = NULL;
    } else {
        al_ptr->values = NULL;
    }
}

static void initialize_sets(universe* u, size_t length) {
    u->sets_length = length;
    u->sets = (set*)malloc(length * sizeof(set));
    for (size_t i = 0; i < length; i++) {
        strcpy(u->sets[i].name, "");
        u->sets[i].key_values.length = 0;
        u->sets[i].key_values.values = NULL;
    }
}

static const char* STRING_TYPE_NAME = "STRING";
static const char* INTEGER_TYPE_NAME = "INTEGER";
static const char* FLOAT_TYPE_NAME = "FLOAT";

int type_name2data_type(const char* type_name) {
    char type_name_uppercase[strlen(type_name) + 1];
    strcpy(type_name_uppercase, type_name);
    str2upper(type_name_uppercase);

    if (strcmp(type_name_uppercase, STRING_TYPE_NAME) == 0)
        return STRING_TYPE;
    if (strcmp(type_name_uppercase, INTEGER_TYPE_NAME) == 0)
        return INTEGER_TYPE;
    if (strcmp(type_name_uppercase, FLOAT_TYPE_NAME) == 0)
        return FLOAT_TYPE;
    return NO_TYPE;
}

const char* data_type2type_name(int data_type) {
    if (data_type == STRING_TYPE)
        return STRING_TYPE_NAME;
    if (data_type == INTEGER_TYPE)
        return INTEGER_TYPE_NAME;
    if (data_type == FLOAT_TYPE)
        return FLOAT_TYPE_NAME;
    return NULL;
}

// Returns the index if exists a set with that name in the universe. MAX_ULONG otherwise.
static size_t universe_set_position(universe* u, const char* set_name) {
    for(size_t i = 0; i < u->sets_length; i++) {
        if (strcmp(set_name, u->sets[i].name) == 0)
            return i;
    }
    return MAX_ULONG;
}

// Returns 1 if exists a set with that name in the universe. 0 otherwise.
static size_t exists_universe_set(universe* u, const char* set_name) {
    return universe_set_position(u, set_name) != MAX_ULONG;
}

// MODIFY UNIVERSE

static void modify_array_list(array_list* a_list, const char** values[], size_t length, size_t data_types_length) {
    char*** copy = copy_string_matrix2d(values, length, data_types_length);
    a_list->length = length;
    a_list->values = copy;
}

static void modify_set(set* s, const char* name, const char*** values, size_t length, size_t key_data_types_length) {
    strcpy(s->name, name);
    modify_array_list(&s->key_values, values, length, key_data_types_length);
}

static void modify_u_set(universe u, size_t set_index, const char* name, const char*** values, size_t length) {
    modify_set(&u.sets[set_index], name, values, length, u.key_data_type_names.length);
}

// CREATE EMPTY UNIVERSE

universe create_empty_universe(void) {
    universe u;
    strcpy(u.name, "");

    initialize_string_list(&u.key_data_type_names, 0);
    initialize_string_list(&u.key_data_names, 0);
    initialize_string_list(&u.attribute_data_type_names, 0);
    initialize_string_list(&u.key_data_names, 0);

    initialize_array_list(&u.key_values, 0);
    initialize_array_list(&u.attribute_values, 0);

    u.sets = NULL;
    u.sets_length = 0;
    
    return u;
}

// FREE MEMORY OF UNIVERSE

void free_string_list(string_list* sl_ptr) {
    if (sl_ptr == NULL) return;
    if (sl_ptr->length == 0 || sl_ptr->strings == NULL) return;

    for(size_t i = 0; i < sl_ptr->length; i++) {
        if (sl_ptr->strings[i] != NULL)
            free(sl_ptr->strings[i]);
    }
    free(sl_ptr->strings);
    sl_ptr->length = 0;
    sl_ptr->strings = NULL;
}

static void free_array_list(array_list* sl_ptr, size_t data_type_length) {
    if (sl_ptr == NULL) return;
    if (sl_ptr->length == 0 || sl_ptr->values == NULL) return;

    for(size_t i = 0; i < sl_ptr->length; i++) {
        if (sl_ptr->values[i] != NULL) {
            for (size_t j = 0; j < data_type_length; j++) {
                if (sl_ptr->values[i][j] != NULL)
                    free(sl_ptr->values[i][j]);
            }
            free(sl_ptr->values[i]);
        }
    }
    free(sl_ptr->values);
    sl_ptr->length = 0;
    sl_ptr->values = NULL;
}

static void free_sets(universe* u, size_t key_data_type_length) {
    if (u == NULL || u->sets == NULL || u->sets_length == 0) return;

    for(size_t i = 0; i < u->sets_length; i++) {
        free_array_list(&u->sets[i].key_values, key_data_type_length);
    }
    free(u->sets);
    u->sets_length = 0;
    u->sets = NULL;
}

void free_universe(universe* u) {
    size_t key_data_type_length = u->key_data_type_names.length;
    size_t attribute_data_type_length = u->attribute_data_type_names.length;

    free_string_list(&u->key_data_type_names);
    free_string_list(&u->key_data_names);
    free_string_list(&u->attribute_data_type_names);
    free_string_list(&u->attribute_data_names);

    free_array_list(&u->key_values, key_data_type_length);
    free_array_list(&u->attribute_values, attribute_data_type_length);

    free_sets(u, key_data_type_length);
}

// CREATE UNIVERSE EXAMPLE

universe create_universe_example(void){   
    universe u = create_empty_universe();
    // Set universe name
	strcpy(u.name, "Animal");

    // Set key and attributes names and types
    const char* key_data_type_names[] = {"STRING"};
    const char* key_data_names[] = {"scientific_name"}; 
    const char* attribute_data_type_names[] = {"STRING", "INTEGER"};
    const char* attribute_data_names[] = {"name", "population"}; 
    modify_string_list(&u.key_data_type_names, key_data_type_names, 1);
    modify_string_list(&u.key_data_names, key_data_names, 1);
    modify_string_list(&u.attribute_data_type_names, attribute_data_type_names, 2);
    modify_string_list(&u.attribute_data_names, attribute_data_names, 2);

    // Set universe keys
    const char* key_value_1[] = {"\"Gallus gallus domesticus\""};
    const char* key_value_2[] = {"\"Chiroptera\""};
    const char* key_value_3[] = {"\"Corvus\""};
    const char** key_values[] = {key_value_1, key_value_2, key_value_3};
    modify_array_list(&u.key_values, key_values, 3, u.key_data_type_names.length);

    // Set universe attributes
    const char* attribute_value_1[] = {"\"Chicken\"", "25000000000"};
    const char* attribute_value_2[] = {"\"Bat\"", "3000000000"};
    const char* attribute_value_3[] = {"\"Crow\"", "100000"};
    const char** attribute_values[] = {attribute_value_1, attribute_value_2, attribute_value_3}; // {{'"crow"', '100000'}, {'"chicken"', '25000000000'}, {'"bat"', '3000000000'}};
    modify_array_list(&u.attribute_values, attribute_values, 3, u.attribute_data_type_names.length);
    
    // We allocate memory for 2 Sets
    u.sets_length = 2;
    u.sets = (set*)malloc(u.sets_length * sizeof(set));
    // We create the Set "Bird"
    const char** set0_values[] = {key_value_1, key_value_3}; // {{"\"Gallus gallus domesticus\""}, {"\"Corvus\""}}
    modify_u_set(u, 0, "Bird", set0_values, 2);
    // We create the Set "CanFly"
    const char** set1_values[] = {key_value_2, key_value_3}; // {{"\"Chiroptera\""}, {"\"Corvus\""}}
    modify_u_set(u, 1, "CanFly", set1_values, 2);

    return u;
}

// PRINT UNIVERSE

static void print_array_list(array_list a_list, size_t data_types_length) {
    printf("[");
    for (size_t i = 0; i < a_list.length; i++) {
        char** strings = a_list.values[i];
        printf("[");
        for (size_t j = 0; j < data_types_length; j++) {
            char* str = strings[j];
            printf("%s", str);
            if (j < data_types_length-1)
                printf(", ");
        }
        printf("]");
        if (i < a_list.length-1)
            printf(", ");
    }
    printf("]");
}

static void print_key_values(universe u) {
    print_array_list(u.key_values, u.key_data_type_names.length);
}

static void print_attribute_values(universe u) {
    print_array_list(u.attribute_values, u.attribute_data_type_names.length);
}

void print_string_list(string_list strings){
    printf("[");
    for (size_t i = 0; i < strings.length - 1; i++) {
        char* str = strings.strings[i];
        printf("\"%s\", ", str);
    }
    if (strings.length > 0) {
        char* str = strings.strings[strings.length-1];
        printf("\"%s\"", str);
    }
    printf("]");
}

static void print_set(universe u, size_t set_index) {
    set* s = &u.sets[set_index];
    printf("Set %s: ", s->name);
    print_array_list(s->key_values, u.key_data_type_names.length);
}

static void print_sets(universe u) {
    for (size_t i = 0; i < u.sets_length; i++) {
        print_set(u, i);
        printf("\n");
    }
}

void print_universe(universe u){ 
    printf("Universe name: %s", u.name);
    printf("\n\n");

    printf("Key data type names: ");
    print_string_list(u.key_data_type_names);
    printf("\n");
    printf("Key data names: ");
    print_string_list(u.key_data_names);
    printf("\n");
    printf("Attribute data type names: ");
    print_string_list(u.attribute_data_type_names);
    printf("\n");
    printf("Attribute data names: ");
    print_string_list(u.attribute_data_names);
    printf("\n\n");

    printf("Key values: ");
    print_key_values(u);
    printf("\n\n");

    printf("Attribute values: ");
    print_attribute_values(u);
    printf("\n\n");

    printf("Sets: \n");
    print_sets(u);
    printf("\n");
}

// UNIVERSE PARSE

// Count the amounts of times a character appears in a string
static size_t count_chars(char* str, char c) {
    size_t count = 0;
    for (int i = 0; str[i]; i++) {
        count += (str[i] == c);
    }
    return count;
}

static int count_lines(char* str) {
    return (int) count_chars(str, '\n');
}

// Count the amounts of times a character delimiter appears in a string (ignoring if they are inside of quoting "")
static size_t count_delimiter(char* str, char delimiter) {
    assert(delimiter != '"');
    size_t count = 0;
    char previous_char = '\0';
    int inside_string = FALSE;
    for (int i = 0; str[i]; i++) {
        char current_char = str[i];
        if (current_char == '"' && (previous_char != '\\'))
            inside_string = !inside_string;
        else if (!inside_string && (current_char == delimiter))
            count += 1;
        previous_char = current_char;
    }
    return count;
}

// Count the amounts of times a character delimiter appears in a string (ignoring if they are inside of quoting "") and ignoring parenthesis content
static size_t count_delimiter_ignoring_parentheses_content(char* str, char delimiter) {
    assert((delimiter != '"') && (delimiter != '(') && (delimiter != ')'));
    size_t count = 0;
    char previous_char = '\0';
    int inside_string = FALSE;
    int inside_parentheses = FALSE;
    for (int i = 0; str[i]; i++) {
        char current_char = str[i];
        if (current_char == '"' && (previous_char != '\\'))
            inside_string = !inside_string;
        else if (!inside_parentheses && (current_char == '('))
            inside_parentheses = TRUE;
        else if (inside_parentheses && (current_char == ')'))
            inside_parentheses = FALSE;
        else if ((!inside_string && !inside_parentheses) && (current_char == delimiter))
            count += 1;
        previous_char = current_char;
    }
    return count;
}

static const char* regex_string_value_type = ",?\\s*(\\w+)\\s+(\\w+)\\s*((,\\s*\\w+\\s+\\w+\\s*)*)";
static regex_t regex_value_type;

static int parse_universe_value_type(universe * u, char* str, char* error_message) {
    size_t maxGroups = 5;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;
    size_t data_type_length = count_delimiter(str, ',') + 1;

    char* key_data_type_names[data_type_length];
    char* key_data_names[data_type_length];
    memset(key_data_type_names, 0, sizeof(key_data_type_names));
    memset(key_data_names, 0, sizeof(key_data_names));

    int error = 0;

    for (size_t i = 0; i < data_type_length; i++) {

        if (regexec(&regex_value_type, cursor, maxGroups, group_array, 0)) {
            strcpy(error_message, "Error parsing the universe definition. Usage: CREATE UNIVERSE <universe_name>(<value_name_1> <data_type_1>, <value_name_2> <data_type_2>);");
            error = 1;
            break;
        }

        for (size_t g = 0; g < maxGroups; g++)
        {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        key_data_names[i] = str_copy(groups[1]);
        key_data_type_names[i] = str_copy(groups[2]);

        size_t offset = group_array[3].rm_so;
        cursor += offset;

        for (size_t g = 0; g < maxGroups; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

    }

    modify_string_list2(&u->key_data_type_names, key_data_type_names, data_type_length);
    modify_string_list2(&u->key_data_names, key_data_names, data_type_length);
    
    for (size_t i = 0; i < data_type_length; i++) {
        if (key_data_names[i] != NULL)
            free(key_data_names[i]);
        if (key_data_type_names[i] != NULL)
            free(key_data_type_names[i]);
    }

    return error;
}

static const char* regex_string_universe_definition = "\\s*CREATE\\s+UNIVERSE\\s+(\\w+)\\s*\\(\\s*(\\w+\\s+\\w+\\s*(,\\s*\\w+\\s+\\w+\\s*)*)\\)\\s*";
static regex_t regex_universe_definition;

// Returns 0 if it is successful
static int parse_universe_definition(universe * u, char* str, char* error_message) {
    size_t maxGroups = 4;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;

    if (regexec(&regex_universe_definition, cursor, maxGroups, group_array, 0)) {
        strcpy(error_message, "Error parsing the universe definition. Usage: CREATE UNIVERSE <universe_name>(<value_name_1> <data_type_1>);");
        return 1;
    }

    for (size_t g = 0; g < maxGroups; g++)
    {
        groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc

    }

    char* universe_name = groups[1];
    if (strlen(universe_name) >= 256) {
        strcpy(error_message, "Error parsing the universe definition: <universe_name> too long (max 256).");
        return 1;
    }
    strcpy(u->name, universe_name);

    if (parse_universe_value_type(u, groups[2], error_message)) {
        return 1;
    }

    // TODO: Check that there are correct data_types and data_names

    for (size_t g = 0; g < maxGroups; g++) {
        if (groups[g] != NULL)
            free(groups[g]);
    }

    return 0;
}

static int parse_attributes_value_type(universe * u, char* str, char* error_message) {
    size_t maxGroups = 5;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;
    size_t data_type_length = count_delimiter(str, ',') + 1;

    char* attribute_data_type_names[data_type_length];
    char* attribute_data_names[data_type_length];
    memset(attribute_data_type_names, 0, sizeof(attribute_data_type_names));
    memset(attribute_data_names, 0, sizeof(attribute_data_names));

    int error = 0;

    for (size_t i = 0; i < data_type_length; i++) {

        if (regexec(&regex_value_type, cursor, maxGroups, group_array, 0)) {
            strcpy(error_message, "Error parsing the attribute values in the attribute definition. Usage: CREATE ATTRIBUTES <universe_name>(<value_name_1> <data_type_1>, <value_name_2> <data_type_2>);");
            error = 1;
            break;
        }

        for (size_t g = 0; g < maxGroups; g++)
        {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        attribute_data_names[i] = str_copy(groups[1]);
        attribute_data_type_names[i] = str_copy(groups[2]);

        size_t offset = group_array[3].rm_so;
        cursor += offset;

        for (size_t g = 0; g < maxGroups; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

    }

    modify_string_list2(&u->attribute_data_type_names, attribute_data_type_names, data_type_length);
    modify_string_list2(&u->attribute_data_names, attribute_data_names, data_type_length);
    
    for (size_t i = 0; i < data_type_length; i++) {
        if (attribute_data_names[i] != NULL)
            free(attribute_data_names[i]);
        if (attribute_data_type_names[i] != NULL)
            free(attribute_data_type_names[i]);
    }

    return error;
}

static const char* regex_string_attributes_definition = "\\s*CREATE\\s+ATTRIBUTES\\s+(\\w+)\\s*\\(\\s*(\\w+\\s+\\w+\\s*(,\\s*\\w+\\s+\\w+\\s*)*)\\)\\s*";
static regex_t regex_attributes_definition;

// Returns 0 if it is successful
static int parse_attributes_definition(universe * u, char* str, char* error_message) {
    size_t maxGroups = 4;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;

    if (regexec(&regex_attributes_definition, cursor, maxGroups, group_array, 0)) {
        strcpy(error_message, "Error parsing the attributes definition. Usage: CREATE ATTRIBUTES <universe_name>(<value_name_1> <data_type_1>);");
        return 1;
    }

    for (size_t g = 0; g < maxGroups; g++)
    {
        groups[g] = NULL;
        if (group_array[g].rm_eo != -1)
            groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
    }

    char* universe_name = groups[1];
    if (strcmp(u->name, universe_name) != 0) {
        strcpy(error_message, "Error parsing the attributes definition: Incorrect <universe_name>");
        return 1;
    }

    if (parse_attributes_value_type(u, groups[2], error_message)) {
        return 1;
    }

    // TODO: Check that there are correct data_types and data_names

    for (size_t g = 0; g < maxGroups; g++) {
        if (groups[g] != NULL)
            free(groups[g]);
    }

    return 0;
}

static const char* regex_string_str_1 = "\\s*\"(([^\"\\]|(\\\\.))+)\"\\s*";
static regex_t regex_str_1;
static const char* regex_string_str_2 = "\\s*\'(([^\'\\]|(\\\\.))+)\'\\s*";
static regex_t regex_str_2;

static char* parse_string(char* string_value) {
    size_t maxGroups = 4;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = string_value;

    if (regexec(&regex_str_1, cursor, maxGroups, group_array, 0) == 0) {
        for (size_t g = 0; g < 2; g++) {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        char* value = str_copy(groups[1]);

        for (size_t g = 0; g < 2; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

        return value;
    }

    /* TOREMOVE
    if (regexec(&regex_str_2, cursor, maxGroups, group_array, 0) == 0) {
        for (size_t g = 0; g < 2; g++) {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        char* value = str_copy(groups[1]);

        for (size_t g = 0; g < 2; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

        return value;
    }*/

    return NULL;
}

static const char* regex_string_int_1 = "\\s*[-]\\s*([0-9]+)\\s*";
static regex_t regex_int_1;
static const char* regex_string_int_2 = "\\s*[+]?\\s*([0-9]+)\\s*";
static regex_t regex_int_2;

static char* parse_integer(char* string_value) {
    size_t maxGroups = 2;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = string_value;

    if (regexec(&regex_int_1, cursor, maxGroups, group_array, 0) == 0) {
        for (size_t g = 0; g < maxGroups; g++) {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        char value_str[strlen(groups[1])+2];
        strcat(strcpy(value_str, "-"), groups[1]);
        char* value = str_copy(value_str);

        for (size_t g = 0; g < maxGroups; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

        return value;
    }

    if (regexec(&regex_int_2, cursor, maxGroups, group_array, 0) == 0) {
        for (size_t g = 0; g < maxGroups; g++) {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        char* value = str_copy(groups[1]);

        for (size_t g = 0; g < maxGroups; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

        return value;
    }

    return NULL;
}

static char* parse_float(char* string_value) {
    // TODO 
    return parse_integer(string_value);
}

static char* parse_value(char* string_value, int data_type) {
    if (data_type == STRING_TYPE)
        return parse_string(string_value);
    if (data_type == INTEGER_TYPE)
        return parse_integer(string_value);
    if (data_type == FLOAT_TYPE)
        return parse_float(string_value);
    return NULL;
}

static const char* any_value = "(([a-zA-Z0-9_.+\\-]+)|(\"(([^\"\\\\]|(\\\\.))+)\"))"; // This has 6 groups (We only use group 1)
static char* regex_string_universe_values; // concat8(",?\\s*", any_value, "\\s*((,\\s*", any_value, "\\s*)*)", NULL, NULL, NULL);
static regex_t regex_universe_values;

static int parse_universe_values(universe * u, char* str, char* error_message, size_t data_type_length, size_t data_index, int is_key_value) {
    size_t maxGroups = 15;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;
    //size_t data_type_length = count_delimiter(str, ',') + 1;
    if (data_type_length != count_delimiter(str, ',') + 1) {
        strcpy(error_message, "Error parsing values: Wrong amount of values");
        return 1;
    }

    char* data_values[data_type_length];
    memset(data_values, 0, sizeof(data_values));

    int error = 0;

    for (size_t i = 0; i < data_type_length; i++) {

        if (regexec(&regex_universe_values, cursor, maxGroups, group_array, 0)) {
            strcpy(error_message, "Error parsing the values.");
            error = 1;
            break;
        }

        for (size_t g = 0; g < maxGroups; g++)
        {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        data_values[i] = str_copy(groups[1]);

        size_t offset = group_array[7].rm_so;
        cursor += offset;

        for (size_t g = 0; g < maxGroups; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

    }

    char* parsed_data_values[data_type_length];
    memset(parsed_data_values, 0, sizeof(parsed_data_values));

    //Parse data

    array_list* values_ptr;
    string_list* data_type_names_ptr;
    if(is_key_value) {
        values_ptr = &(u->key_values);
        data_type_names_ptr = &(u->key_data_type_names);
    } else {
        values_ptr = &(u->attribute_values);
        data_type_names_ptr = &(u->attribute_data_type_names);
    }

    assert(data_index < values_ptr->length);
    assert(data_type_length == data_type_names_ptr->length);

    for (size_t i = 0; i < data_type_length; i++) {
        int data_type = type_name2data_type(data_type_names_ptr->strings[i]);
        parsed_data_values[i] = parse_value(data_values[i], data_type);
        if (parsed_data_values[i] == NULL) {
            if (is_key_value)
                sprintf(error_message, "Error parsing key value %s of type %s.", data_values[i], data_type_names_ptr->strings[i]);
            else
                sprintf(error_message, "Error parsing attribute value %s of type %s.", data_values[i], data_type_names_ptr->strings[i]);
            error = 1;
            break;
        }
    }

    if (!error)
        values_ptr->values[data_index] = copy_string_array2(parsed_data_values, data_type_length);
    
    for (size_t i = 0; i < data_type_length; i++) {
        if (data_values[i] != NULL)
            free(data_values[i]);
        if (parsed_data_values[i] != NULL)
            free(parsed_data_values[i]);
    }

    return error;
}

static char* regex_string_universe_insert_supp; // concat16("\\s*((", any_value, ")|(\\(", any_value, "(\\s*,\\s*", any_value, ")*\\)))\\s*:"
                                                //        , "\\s*((", any_value, ")|(\\(", any_value, "(\\s*,\\s*", any_value, ")*\\)))\\s*", NULL, NULL);
static regex_t regex_universe_insert_supp;

// Returns 0 if it is successful
static int parse_universe_insert_supp(universe * u, char* str, char* error_message) {
    size_t maxGroups = 23;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;

    size_t data_length = count_delimiter(str, ':');

    assert(u->key_values.length == u->attribute_values.length);

    size_t first_data_index = 0;
    if (u->key_values.length == 0) {
        initialize_array_list(&(u->key_values), data_length); // malloc
        initialize_array_list(&(u->attribute_values), data_length); // malloc
    // If there are already values we keep them
    } else {
        size_t previous_data_length = u->key_values.length;
        data_length = previous_data_length + data_length;

        char*** previous_key_values = u->key_values.values;
        char*** previous_attribute_values = u->attribute_values.values;

        initialize_array_list(&(u->key_values), data_length); // malloc
        initialize_array_list(&(u->attribute_values), data_length); // malloc

        for (size_t i = 0; i < previous_data_length; i++) {
            u->key_values.values[i] = previous_key_values[i];
            u->attribute_values.values[i] = previous_attribute_values[i];
        }

        free(previous_key_values);
        free(previous_attribute_values);

        first_data_index = previous_data_length;
    }

    int is_key_value = 1;
    int is_attribute_value = 0;

    int error = 0;

    for (size_t i = first_data_index; i < data_length; i++) {
        size_t data_index = i;

        // Parse key values

        if (regexec(&regex_universe_insert_supp, cursor, maxGroups, group_array, 0)) {
            sprintf(error_message, "Error parsing key values in row %zu at insert supp.", i);
            error = 1;
            break;
        }

        for (size_t g = 0; g < maxGroups; g++)
        {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        char* key_values_string = str_copy(groups[1]);
        size_t offset = group_array[0].rm_eo;

        for (size_t g = 0; g < maxGroups; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

        if (!cursor[offset-1] || cursor[offset] != ':') {
            sprintf(error_message, "Error parsing values in row %zu at insert supp. Expected character ':' missing.", i);
            error = 1;
            break;
        }

        if (parse_universe_values(u, key_values_string, error_message, u->key_data_names.length, data_index, is_key_value)) {
            // strcpy(error_message, "Error parsing key values.");
            error = 1;
        }

        free(key_values_string);

        cursor += offset+1; // The pointer ^<keys>:<attributes> moves towards <keys>:^<attributes>

        if (error)
            break;

        // Parse attribute values

        if (regexec(&regex_universe_insert_supp, cursor, maxGroups, group_array, 0)) {
            sprintf(error_message, "Error parsing attribute values in row %zu at insert supp.", i);
            error = 1;
            break;
        }

        for (size_t g = 0; g < maxGroups; g++)
        {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        char* attribute_values_string = str_copy(groups[1]);
        offset = group_array[0].rm_eo;

        for (size_t g = 0; g < maxGroups; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

        if (parse_universe_values(u, attribute_values_string, error_message, u->attribute_data_names.length, data_index, is_attribute_value)) {
            // strcpy(error_message, "Error parsing attribute values.");
            error = 1;
        }

        free(attribute_values_string );

        cursor += offset;

        if (error)
            break;
    }

    return error;
}

static const char* regex_string_universe_insert = "\\s*INSERT\\s*\\{([^;]+)\\}\\s*INTO\\s+(\\w+)\\s*";
static regex_t regex_universe_insert;

// Returns 0 if it is successful
static int parse_universe_insert(universe * u, char* str, char* error_message) {
    size_t maxGroups = 3;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;

    if (regexec(&regex_universe_insert, cursor, maxGroups, group_array, 0)) {
        strcpy(error_message, "Error parsing the universe insert.");
        return 1;
    }

    for (size_t g = 0; g < maxGroups; g++)
    {
        groups[g] = NULL;
        if (group_array[g].rm_eo != -1)
            groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
    }

    int error = 0;

    char* universe_name = groups[2];
    if (strcmp(u->name, universe_name) != 0) {
        strcpy(error_message, "Error parsing the universe insert: Incorrect <universe_name>");
        error = 1;
    }

    if (!error && parse_universe_insert_supp(u, groups[1], error_message)) {
        // error_message
        error = 1;
    }

    for (size_t g = 0; g < maxGroups; g++) {
        if (groups[g] != NULL)
            free(groups[g]);
    }

    return error;
}

static const char* regex_string_create_set = "^\\s*CREATE\\s+SET\\s+([a-zA-Z0-9_]+)\\s*$";
static regex_t regex_create_set;

// Returns 0 if it is successful
static int parse_create_set(universe * u, char* str, char* error_message, size_t set_index) {
    size_t maxGroups = 2;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;

    if (regexec(&regex_create_set, cursor, maxGroups, group_array, 0)) {
        strcpy(error_message, "Error parsing the create set.");
        return 1;
    }

    for (size_t g = 0; g < maxGroups; g++)
    {
        groups[g] = NULL;
        if (group_array[g].rm_eo != -1)
            groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
    }

    char* set_name = groups[1];
    assert(strlen(set_name) < 256);

    int error = 0;

    if (strcmp(u->name, set_name) == 0) {
        sprintf(error_message, "Error creating set. Sets cannot have the same name as the universe.");
        error = 1;
    }

    if (exists_universe_set(u, set_name)) {
        sprintf(error_message, "Error creating set. Set \"%s\" already exists.", set_name);
        error = 1;
    }

    if (!error) {
        assert(set_index < u->sets_length);
        strcpy(u->sets[set_index].name, set_name);
    }

    for (size_t g = 0; g < maxGroups; g++) {
        if (groups[g] != NULL)
            free(groups[g]);
    }

    return error;
}

static int parse_set_values(universe * u, char* str, char* error_message, size_t data_index, size_t set_index) {
    size_t maxGroups = 15;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;

    size_t data_type_length = u->key_data_type_names.length;
    // size_t data_type_length = count_delimiter(str, ',') + 1;
    if (data_type_length != count_delimiter(str, ',') + 1) {
        strcpy(error_message, "Error parsing set values: Wrong amount of values");
        return 1;
    }

    char* data_values[data_type_length];
    memset(data_values, 0, sizeof(data_values));

    int error = 0;

    for (size_t i = 0; i < data_type_length; i++) {

        if (regexec(&regex_universe_values, cursor, maxGroups, group_array, 0)) {
            strcpy(error_message, "Error parsing the values.");
            error = 1;
            break;
        }

        for (size_t g = 0; g < maxGroups; g++)
        {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        data_values[i] = str_copy(groups[1]);

        size_t offset = group_array[7].rm_so;
        cursor += offset;

        for (size_t g = 0; g < maxGroups; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

    }

    char* parsed_data_values[data_type_length];
    memset(parsed_data_values, 0, sizeof(parsed_data_values));

    //Parse data

    assert(data_index < u->key_values.length);
    assert(data_type_length == u->key_data_type_names.length);
    assert(set_index < u->sets_length);

    for (size_t i = 0; i < data_type_length; i++) {
        int data_type = type_name2data_type(u->key_data_type_names.strings[i]);
        parsed_data_values[i] = parse_value(data_values[i], data_type);
        if (parsed_data_values[i] == NULL) {
            sprintf(error_message, "Error parsing key value %s of type %s.", data_values[i], u->key_data_type_names.strings[i]);
            error = 1;
            break;
        }
    }

    if (!error)
        u->sets[set_index].key_values.values[data_index] = copy_string_array2(parsed_data_values, data_type_length); 
    
    for (size_t i = 0; i < data_type_length; i++) {
        if (data_values[i] != NULL)
            free(data_values[i]);
        if (parsed_data_values[i] != NULL)
            free(parsed_data_values[i]);
    }

    return error;
}

// Returns 0 if it is successful
static int parse_set_insert_supp(universe * u, char* str, char* error_message, size_t set_index) {
    size_t maxGroups = 23;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;

    size_t current_set_data_length = count_delimiter_ignoring_parentheses_content(str, ',') + 1;

    // Assert that the inserted values are less or equal to the amount of values in the universe
    assert(current_set_data_length <= u->key_values.length);

    size_t first_data_index = 0;
    if (u->key_values.length == 0) {
        initialize_array_list(&(u->sets[set_index].key_values), current_set_data_length); // malloc
    // If there are already values we keep them
    } else {
        size_t previous_data_length = u->sets[set_index].key_values.length;
        current_set_data_length = previous_data_length + current_set_data_length;

        char*** previous_key_values = u->sets[set_index].key_values.values;

        initialize_array_list(&(u->sets[set_index].key_values), current_set_data_length); // malloc

        for (size_t i = 0; i < previous_data_length; i++)
            u->sets[set_index].key_values.values[i] = previous_key_values[i];

        free(previous_key_values);

        first_data_index = previous_data_length;
    }

    int error = 0;

    for (size_t i = first_data_index; i < current_set_data_length; i++) {
        size_t data_index = i;

        // Parse key values

        if (regexec(&regex_universe_insert_supp, cursor, maxGroups, group_array, 0)) {
            sprintf(error_message, "Error parsing key values in row %zu at insert set supp.", i);
            error = 1;
            break;
        }

        for (size_t g = 0; g < maxGroups; g++)
        {
            groups[g] = NULL;
            if (group_array[g].rm_eo != -1)
                groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        }

        char* key_values_string = str_copy(groups[1]);
        size_t offset = group_array[0].rm_eo;

        for (size_t g = 0; g < maxGroups; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

        if (parse_set_values(u, key_values_string, error_message, data_index, set_index)) {
            // strcpy(error_message, "Error parsing key values.");
            error = 1;
        }

        cursor += offset;

        free(key_values_string);

        if (error)
            return error;
        
    }

    return error;
}

static const char* regex_string_set_insert = "^\\s*INSERT\\s*\\{([^;]+)\\}\\s*INTO\\s+([a-zA-Z0-9_]+)\\s*$";
static regex_t regex_set_insert;

// Returns 0 if it is successful
static int parse_set_insert(universe * u, char* str, char* error_message) {
    size_t maxGroups = 3;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;

    if (regexec(&regex_universe_insert, cursor, maxGroups, group_array, 0)) {
        strcpy(error_message, "Error parsing the set insert.");
        return 1;
    }

    for (size_t g = 0; g < maxGroups; g++)
    {
        groups[g] = NULL;
        if (group_array[g].rm_eo != -1)
            groups[g] = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
    }

    int error = 0;

    char* set_name = groups[2];
    size_t set_index = universe_set_position(u, set_name);
    if (set_index == MAX_ULONG) {   // If set does not exist
        sprintf(error_message, "Error parsing set insert. Set \"%s\" does not exist", set_name);
        error = 1;
    }

    if (!error) {
        if (parse_set_insert_supp(u, groups[1], error_message, set_index))
            error = 1;
    }

    for (size_t g = 0; g < maxGroups; g++) {
        if (groups[g] != NULL)
            free(groups[g]);
    }

    return error;
}

static int regex_is_initialized = 0;

// Compile all the regex expressions that will be used in the parsing
static void initialize_regex(void) {
    if (regex_is_initialized) return;
    // Universe key values and attributes definition
    assert(regcomp(&regex_value_type, regex_string_value_type, REG_EXTENDED | REG_ICASE) == 0);
    assert(regcomp(&regex_universe_definition, regex_string_universe_definition, REG_EXTENDED | REG_ICASE) == 0);
    assert(regcomp(&regex_attributes_definition, regex_string_attributes_definition, REG_EXTENDED | REG_ICASE) == 0);
    // Universe insert
    assert(regcomp(&regex_str_1, regex_string_str_1, REG_EXTENDED) == 0);
    assert(regcomp(&regex_str_2, regex_string_str_2, REG_EXTENDED) == 0);
    assert(regcomp(&regex_int_1, regex_string_int_1, REG_EXTENDED) == 0);
    assert(regcomp(&regex_int_2, regex_string_int_2, REG_EXTENDED) == 0);
    regex_string_universe_values = str_concat(5,"^\\s*[(,]?\\s*", any_value, "\\s*((,\\s*", any_value, "\\s*)*)");
    assert(regcomp(&regex_universe_values, regex_string_universe_values, REG_EXTENDED | REG_ICASE) == 0);
    /*regex_string_universe_insert_supp = str_concat(14, "^,?\\s*((", any_value, ")|(\\(", any_value, "(\\s*,\\s*", any_value, ")*\\)))\\s*:"
                                                       , "\\s*((", any_value, ")|(\\(", any_value, "(\\s*,\\s*", any_value, ")*\\)))\\s*((,[^,]+)*)");*/
    regex_string_universe_insert_supp = str_concat(7, "^,?\\s*((", any_value, ")|(\\(", any_value, "(\\s*,\\s*", any_value, ")*\\)))\\s*");
    assert(regcomp(&regex_universe_insert_supp, regex_string_universe_insert_supp, REG_EXTENDED | REG_ICASE) == 0);
    assert(regcomp(&regex_universe_insert, regex_string_universe_insert, REG_EXTENDED | REG_ICASE) == 0);
    // Create set
    assert(regcomp(&regex_create_set, regex_string_create_set, REG_EXTENDED | REG_ICASE) == 0);
    assert(regcomp(&regex_set_insert, regex_string_set_insert, REG_EXTENDED | REG_ICASE) == 0);

    regex_is_initialized = 1;
}

// Returns 0 if it is successful
int parse_initialization(universe* u, const char* file_name) {
    char error_message[200];
    initialize_regex();

    FILE* ptr = fopen(file_name, "r");
    assert(ptr != NULL); // Check that the file exists

    size_t buffer_size = 1024*1024;
    char* buffer = (char*) malloc(buffer_size * sizeof(char));
    int line = 1;

    if (fscanf(ptr, "%[^;];", buffer) == 0) {
        printf("Error: No semicolons found\n");
        return 1;
    }

    if (parse_universe_definition(u, buffer, error_message)) {
        printf("<Lines %u-%u>: %s\n", line, line+count_lines(buffer), error_message);
        return 1;
    }

    line += count_lines(buffer);

    if (fscanf(ptr, "%[^;];", buffer) == 0) {
        printf("Error: No semicolons found after CREATE UNIVERSE\n");
        return 1;
    }

    if (parse_attributes_definition(u, buffer, error_message)) {
        printf("<Lines %u-%u>: %s\n", line, line+count_lines(buffer), error_message);
        return 1;
    }

    line += count_lines(buffer);

    if (fscanf(ptr, "%[^;];", buffer) == 0) {
        printf("Error: No semicolons found after CREATE ATTRIBUTES\n");
        return 1;
    }

    if (parse_universe_insert(u, buffer, error_message)) {
        printf("<Lines %u-%u>: %s\n", line, line+count_lines(buffer), error_message);
        return 1;
    }

    line += count_lines(buffer);

    int scanned_commands = 3;   // Commands that have been analyzed and executed by now

    // We check if there are more inserts
    while (fscanf(ptr, "%[^;];", buffer) == 1) {
        // if not a universe insert
        if (parse_universe_insert(u, buffer, error_message))
            break;
        scanned_commands += 1;
        line += count_lines(buffer);
    }

    // Count created sets
    size_t sets_count = 0;
    do {

        // if CREATE SET
        if (regexec(&regex_create_set, buffer, 0, NULL, 0) == 0) {
            sets_count += 1;
        }

    } while (fscanf(ptr, "%[^;];", buffer) == 1);
    fclose(ptr);

    // Get the file pointer back to the same position as before
    ptr = fopen(file_name, "r");
    for (int i = 0; i <  scanned_commands; i++)
        fscanf(ptr, "%[^;];", buffer);

    initialize_sets(u, sets_count);

    size_t set_index = 0;

    while (fscanf(ptr, "%[^;];", buffer) == 1) {

        //if CREATE SET
        if (regexec(&regex_create_set, buffer, 0, NULL, 0) == 0) {

            if(parse_create_set(u, buffer, error_message, set_index)) {
                printf("<Lines %u-%u>: %s\n", line, line+count_lines(buffer), error_message);
                return 1;
            }
            
            set_index += 1;
        } else {
            if(parse_set_insert(u, buffer, error_message)) {
                printf("<Lines %u-%u>: %s\n", line, line+count_lines(buffer), error_message);
                return 1;
            }
        }

        line += count_lines(buffer);
    }

    fclose(ptr);

    free(buffer);

    return 0;
}