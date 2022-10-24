#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <regex.h>

static const char* OP_COMPLEMENT_STRING = "COMPLEMENT";
static const char* OP_UNION_STRING = "UNION";
static const char* OP_INTERSECTION_STRING = "INTERSECTION";
static const char* OP_DIFFERENCE_STRING = "DIFFERENCE";

// SUPPORT FUNCTIONS

// Count the amounts of times a character appears in a string
static size_t count_chars(char* str, char c) {
    size_t count = 0;
    for (int i = 0; str[i]; i++) {
        count += (str[i] == c);
    }
    return count;
}

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

static char** copy_string_array(char** strings, size_t length) {
    char** copy = (char**)malloc(length * sizeof(char*));
    for (size_t i = 0; i < length; i++) {
        copy[i] = (char*)malloc((strlen(strings[i])+1) * sizeof(char));
        strcpy(copy[i], strings[i]);
    }
    return copy;
}

static void modify_string_list(string_list* str_list, char** strings, size_t length) {
    char** copy = copy_string_array(strings, length);
    str_list->length = length;
    str_list->strings = copy;
}

// It modifies the string to uppercase
static void str2upper(char* str) {
    for (size_t i = 0; str[i]!='\0'; i++) {
        str[i] = toupper(str[i]);
    }
}

// Returns the index of the delimiter character and ignoring parenthesis content. Returns -1 if not found
static int find_delimiter_ignoring_parentheses_content(char* str, char delimiter) {
    assert((delimiter != '(') && (delimiter != ')'));
    int parentheses_count = 0;
    for (int i = 0; str[i]; i++) {
        char current_char = str[i];
        if (current_char == '(')
            parentheses_count += 1;
        else if (current_char == ')')
            parentheses_count += -1;
        else if ((current_char == delimiter) && (parentheses_count == 0))
            return i;
        
        if (parentheses_count < 0)
            return -1;
    }
    return -1;
}

// SET OPERATION FUNCTIONS

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

static const char* op_type2string(int op_type) {
    if (op_type == OP_COMPLEMENT)
        return OP_COMPLEMENT_STRING;
    if (op_type == OP_UNION)
        return OP_UNION_STRING;
    if (op_type == OP_INTERSECTION)
        return OP_INTERSECTION_STRING;
    if (op_type == OP_DIFFERENCE)
        return OP_DIFFERENCE_STRING;
    return NULL;
}

static int string2op_type(const char* string) {
    if (strcmp(string, OP_COMPLEMENT_STRING) == 0)
        return OP_COMPLEMENT;
    if (strcmp(string, OP_UNION_STRING) == 0)
        return OP_UNION;
    if (strcmp(string, OP_INTERSECTION_STRING) == 0)
        return OP_INTERSECTION;
    if (strcmp(string, OP_DIFFERENCE_STRING) == 0)
        return OP_DIFFERENCE;
    return 0;
}

static void print_set_op(set_op* op) {
    assert(op != NULL);
    if (op->is_leave) {
        printf("%s", op->set_name);
    } else if (op->op_type == OP_COMPLEMENT) {
        printf("set_op{");
        printf("%s ", op_type2string(op->op_type));
        print_set_op(op->left_op);
        printf("}");
    } else {
        printf("set_op{");
        print_set_op(op->left_op);
        printf(" %s ", op_type2string(op->op_type));
        print_set_op(op->right_op);
        printf("}");
    }
}

// REGEX STAFF

static const char* regex_string_column_names = "^,?\\s*([a-zA-Z0-9_]+)\\s*((,\\s*[a-zA-Z0-9_]+\\s*)*)\\s*$";
static regex_t regex_column_names;

static int parse_column_names(query* q, char* str, char* error_message) {
    size_t maxGroups = 4;
    regmatch_t group_array[maxGroups];
    char* groups[maxGroups];
    char* cursor = str;
    size_t length = count_chars(str, ',') + 1;

    char* data_values[length];
    memset(data_values, 0, sizeof(data_values));

    int error = 0;

    for (size_t i = 0; i < length; i++) {

        if (regexec(&regex_column_names, cursor, maxGroups, group_array, 0)) {
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

        size_t offset = group_array[2].rm_so;

        for (size_t g = 0; g < maxGroups; g++) {
            if (groups[g] != NULL)
                free(groups[g]);
        }

        cursor += offset;

    }

    modify_string_list(&q->column_names, data_values, length);

    for (size_t i = 0; i < length; i++) {
        if (data_values[i] != NULL)
            free(data_values[i]);
    }

    return error;
}

static const char* regex_string_parentheses = "^\\s*\\(([^;]+)\\)\\s*$";
static regex_t regex_parentheses;
static const char* regex_string_set_name = "^\\s*([a-zA-Z0-9_]+)\\s*$";
static regex_t regex_set_name;
static const char* regex_string_set_complement_1 = "^\\s*COMPLEMENT\\s+([a-zA-Z0-9_]+)\\s*$";
static regex_t regex_set_complement_1;
static const char* regex_string_set_complement_2 = "^\\s*COMPLEMENT\\s*\\(([^;]+)\\)\\s*$";
static regex_t regex_set_complement_2;
static const char* regex_string_set_binary_op = "^\\s*([a-zA-Z]+)\\s*\\(([^;]+)\\)\\s*$";
static regex_t regex_set_binary_op;

// Returns NULL if error
static set_op* parse_set_operation(const char* string, char* error_message) {
    size_t maxGroups = 3;
    regmatch_t group_array[maxGroups];
    const char* cursor = string;

    if (regexec(&regex_parentheses, cursor, maxGroups, group_array, 0) == 0) {
        size_t g = 1;
        char * string_without_parentheses = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        set_op* op = parse_set_operation(string_without_parentheses, error_message);
        free(string_without_parentheses);
        return op;
    }

    if (regexec(&regex_set_name, cursor, maxGroups, group_array, 0) == 0) {
        size_t g = 1;
        char * set_name = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        return create_set_op_leave(set_name);
    }

    if ((regexec(&regex_set_complement_1, cursor, maxGroups, group_array, 0) == 0)
        || (regexec(&regex_set_complement_2, cursor, maxGroups, group_array, 0) == 0)) {
        size_t g = 1;
        char * content = str_copy_idx(cursor, group_array[g].rm_so, group_array[g].rm_eo); // malloc
        set_op* op = parse_set_operation(content, error_message);
        free(content);
        if (op == NULL)
            return NULL;
        return create_set_op_complement(op, NULL);
    }

    if (regexec(&regex_set_binary_op, cursor, maxGroups, group_array, 0) == 0) {
        int error = 0;
        set_op* op = NULL;
        char* op_type_string = str_copy_idx(cursor, group_array[1].rm_so, group_array[1].rm_eo); // malloc
        char* content = str_copy_idx(cursor, group_array[2].rm_so, group_array[2].rm_eo); // malloc
        // We modify to operation type string to uppercase
        str2upper(op_type_string);
        int op_type = string2op_type(op_type_string);
        // If it does not match a type
        if (op_type == 0) {
            sprintf(error_message, "Error parsing: \"%s\" is not a set operation (COMPLEMENT, UNION, INTERSECTION, DIFFERENCE).", op_type_string);
            error = 1;
        }

        int split_index = find_delimiter_ignoring_parentheses_content(content, ',');

        if (!error && split_index == -1) {
            sprintf(error_message, "Error parsing: Missing comma. Expected format: %s (<op_set>, <op_set)", op_type_string);
            error = 1;
        }

        if (!error) {
            // We split the string in two where the comma is
            content[split_index] = '\0';
            char* left_op_string = content;
            char* right_op_string = &content[split_index+1];

            set_op* left_op = parse_set_operation(left_op_string, error_message);
            if (left_op != NULL) {
                set_op* right_op = parse_set_operation(right_op_string, error_message);
                if (right_op != NULL)
                    op = create_set_op(left_op, right_op, op_type);
            }
        }

        free(op_type_string);
        free(content);

        return op;
    }

    sprintf(error_message, "Error parsing string=%s", string);

    return NULL;
}

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

    int error = 0;

    char* column_names = groups[1];
    char* set_operation = groups[3];

    printf("column_names=%s\n", column_names);  // TOREMOVE

    if (parse_column_names(q, column_names, error_message)) {
        strcpy(error_message, "Error parsing column names in the query.");
        error = 1;
    }

    for (size_t i = 0; i < q->column_names.length; i++) {
        printf("Value %zu:\"%s\"\n", i, q->column_names.strings[i]);    // TOREMOVE
    }

    printf("set_operation=%s\n", set_operation);  // TOREMOVE

    q->op = parse_set_operation(set_operation, error_message);

    if (q->op != NULL) 
        {print_set_op(q->op);  printf("\n");} // TOREMOVE
    else
        printf("%s\n", error_message); // TOREMOVE

    if (q->op == NULL) {
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
    
    assert(regcomp(&regex_column_names, regex_string_column_names, REG_EXTENDED | REG_ICASE) == 0);
    assert(regcomp(&regex_query, regex_string_query, REG_EXTENDED | REG_ICASE) == 0);

    assert(regcomp(&regex_parentheses, regex_string_parentheses, REG_EXTENDED | REG_ICASE) == 0);
    assert(regcomp(&regex_set_name, regex_string_set_name, REG_EXTENDED | REG_ICASE) == 0);
    assert(regcomp(&regex_set_complement_1, regex_string_set_complement_1, REG_EXTENDED | REG_ICASE) == 0);
    assert(regcomp(&regex_set_complement_2, regex_string_set_complement_2, REG_EXTENDED | REG_ICASE) == 0);
    assert(regcomp(&regex_set_binary_op, regex_string_set_binary_op, REG_EXTENDED | REG_ICASE) == 0);

    regex_is_initialized = 1;
}

// If succesful returns 0 and modifies the query
int parse_query(query* q, const char* query_string) {
    initialize_regex();
    char error_message[200];
    if (parse_entire_query(q, query_string, error_message)) {
        printf("%s\n", error_message);
        return 1;
    }

    return 0;
}