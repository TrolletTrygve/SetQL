#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char** copy_string_array(const char* strings[], size_t length) {
    char** copy = (char**)malloc(length * sizeof(char*));
    for (size_t i = 0; i < length; i++) {
        copy[i] = (char*)malloc(strlen(strings[i]) * sizeof(char));
        strcpy(copy[i], strings[i]);
    }
    return copy;
}

static void modify_string_list(string_list* str_list, const char** strings, size_t length) {
    char** copy = copy_string_array(strings, length);
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