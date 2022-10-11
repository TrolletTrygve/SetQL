#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <regex.h>

static char* str_copy(char* str) {
    char* new_str = (char*) malloc(strlen(str + 1) * sizeof(char));
    strcpy(new_str, str);
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
        copy[i] = (char*)malloc((strlen(strings[i])+1) * sizeof(char));
        strcpy(copy[i], strings[i]);
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

universe create_universe_example(void){     // TODO
    universe u;
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

void print_universe(universe u){            // TODO
    printf("Universe name: %s", u.name);
    printf("\n\n");

    printf("Key data type names: ");
    print_string_list(u.key_data_type_names);
    printf("\n");
    printf("Key data names: ");
    print_string_list(u.key_data_names);
    printf("\n");
    if (1) return;
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


// Count the amounts of times a character apperas in a string
static int count_chars(char* str, char c) {
    int count = 0;
    for (int i = 0; str[i]; i++) {
        count += (str[i] == c);
    }
    return count;
}

static int count_lines(char* str) {
    return count_chars(str, '\n');
}

static const char* regex_string_value_type = ",?\\s*(\\w+)\\s+(\\w+)\\s*((,\\s*\\w+\\s+\\w+\\s*)*)";
static regex_t regex_value_type;

static int parse_value_type(universe * u, char* str, char* error_message) {
    size_t maxGroups = 4;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;
    size_t data_type_length = count_chars(str, ',') + 1;

    char* key_data_type_names[data_type_length];
    char* key_data_names[data_type_length];

    for (size_t i = 0; i < data_type_length; i++) {

        if (regexec(&regex_value_type, cursor, maxGroups, group_array, 0)) {
            strcpy(error_message, "Error parsing the universe definition. Usage: CREATE UNIVERSE <universe_name>(<value_name_1> <data_type_1>, <value_name_2> <data_type_2>);");
            return 1;
        }

        for (size_t g = 0; g < maxGroups; g++)
        {
            char cursorCopy[strlen(cursor) + 1];
            strcpy(cursorCopy, cursor);
            cursorCopy[group_array[g].rm_eo] = '\0';
            char* group_str = cursorCopy + group_array[g].rm_so;
            groups[g] = str_copy(group_str); // malloc
        }

        key_data_names[i] = str_copy(groups[1]);
        key_data_type_names[i] = str_copy(groups[2]);

        size_t offset = group_array[3].rm_so;
        cursor += offset;

        for (size_t g = 0; g < maxGroups; g++)
            free(groups[g]);

    }

    modify_string_list2(&u->key_data_type_names, key_data_type_names, data_type_length);
    modify_string_list2(&u->key_data_names, key_data_names, data_type_length);
    
    for (size_t i = 0; i < data_type_length; i++) {
        
        free(key_data_names[i]);
        free(key_data_type_names[i]);
    }

    return 0;
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
        char cursorCopy[strlen(cursor) + 1];
        strcpy(cursorCopy, cursor);
        cursorCopy[group_array[g].rm_eo] = '\0';
        char* group_str = cursorCopy + group_array[g].rm_so;
        groups[g] = str_copy(group_str);
        /*TOREMOVE
        printf("Group %u: [%2u-%2u]: %s\n",
                g, group_array[g].rm_so, group_array[g].rm_eo,
                groups[g]);*/ 

    }

    char* universe_name = groups[1];
    if (strlen(universe_name) >= 50) {
        strcpy(error_message, "Error parsing the universe definition: <universe_name> too long (max 50).");
        return 1;
    }
    strcpy(u->name, universe_name);

    if (parse_value_type(u, groups[2], error_message)) {
        return 1;
    }

    // TODO: Check that there are correct data_types and data_names

    for (size_t g = 0; g < maxGroups; g++)
        free(groups[g]);

    return 0;
}

// Compile all the regex expressions that will be used in the parsing
static void initialize_regex(void) {
    assert(regcomp(&regex_value_type, regex_string_value_type, REG_EXTENDED) == 0);
    assert(regcomp(&regex_universe_definition, regex_string_universe_definition, REG_EXTENDED) == 0);
}

// Returns 0 if it is successful
int parse_initialization(universe* u, const char* file_name) {
    char error_message[200];
    initialize_regex();

    FILE* ptr = fopen(file_name, "r");
    assert(ptr != NULL); // Check that the file exists

    size_t buffer_size = 1024;
    char* buffer = (char*) malloc(buffer_size * sizeof(char));
    int line = 1;

    if (fscanf(ptr, "%[^;];", buffer) == 0) {
        printf("Error: No semicolons found\n");
        return 1;
    }

    if (parse_universe_definition(u, buffer, error_message)) {
        printf("<Lines %u-%u>: %s", line, line+count_lines(buffer), error_message);
        return 1;
    }

    line += count_lines(buffer);

    /* TOREMOVE
    while (fscanf(ptr, "%[^;];", buffer) == 1) {
        printf("<LINE %i>", line);
        printf("%s<SEMICOLON>", buffer);


        line += count_lines(buffer);
    }*/

    free(buffer);

    return 0;
}