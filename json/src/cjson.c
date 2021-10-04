/*************************************************************************
	> File Name: include/cjson.h
	> Author: racle
	> Mail: racleray@qq.com
	> Created Time: Wed 25 Aug 2021 06:25:21 PM CST
 ************************************************************************/

#include "cjson.h"

#define CONST_BIT 256

enum {
    J_NULL = 0,
    J_True,
    J_False,
    J_Int,
    J_Double,
    J_String,
    J_Array,
    J_Object
};

static const char *ep;

static inline void *emalloc(size_t size);
static inline void safe_free(void *ptr);
static inline void error_exit(int status, const char *error_msg);
static int strcmp_case(const char *s1, const char *s2);
static const char *skip_invalid(const char *value);

static JNODE_p new_node(void);

static const char *parse_value(JNODE_p node, const char *value);
static const char *parse_string(JNODE_p node, const char *value);
static const char *parse_number(JNODE_p node, const char *value);
static const char *parse_array(JNODE_p node, const char *value);
static const char *parse_object(JNODE_p node, const char *value);

static char *print_const(const char *str);
static char *print_value(JNODE_p node, int depth);
static char *print_number(JNODE_p node);
static char *print_string_base(const char *str);
static char *print_string(JNODE_p node);
static char *print_array(JNODE_p node, int depth);
static char *print_object(JNODE_p node, int depth);

static void show_search_result(JNODE_p node, const char *name);

/* Functions for input and output */
JNODE_p text_to_json(char *text) {
    JNODE_p json = NULL;
    json = json_parse(text);
    if (!json) {
        printf("Parsing input failed. Error before: %s \n", ep);
        exit(EXIT_FAILURE);
    }
    return json;
}

JNODE_p read_json(char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) error_exit(2, "Failed to open input file. \n");

    int len = 0;
    char tmp = 0;
    while (tmp != EOF) {
        tmp = fgetc(fp);
        len++;
    }
    if (ferror(fp)) {
        fclose(fp); // locate error
        error_exit(3, "Failed to read input file. \n");
    }
    rewind(fp);

    char *text = (char *)emalloc(sizeof(char) * (len + 1));
    if (fread(text, sizeof(char), len, fp) != (len - 1)) {
        safe_free(text);
        fclose(fp);
        error_exit(3, "Failed to read input file. \n");
    }

    JNODE_p json = text_to_json(text);
    safe_free(text);
    return json;
}

void output_json(JNODE_p root, FILE *out) {
    char *text = json_format(root);
    if (!text) {
        safe_free(text);
        error_exit(3, "Failed to foramt json object. \n");
    }
    fprintf(out, "%s \n", text);
    safe_free(text);
}

/* Functions for parsing text to json */
JNODE_p json_parse(const char *value) {
    const char *end = NULL;
    JNODE_p c = new_node();
    ep = 0;
    end = parse_value(c, skip_invalid(value));
    if (!end) {
        json_delete(c);
        return NULL;
    }
    return c;
}

void json_delete(JNODE_p root) {
    JNODE_p next;
    while (root) {
        next = root->next;
        if (root->child) json_delete(root->child);
        if ((root->type == J_String) && root->value.string_val)
            safe_free(root->value.string_val);
        if (!(root->type & CONST_BIT) && root->name);
        safe_free(root->name);
        safe_free(root);
        root = next;
    }
}

/* Function for formating json struct, return char* to arr print function */
char *json_format(JNODE_p root) {
    return print_value(root, 0);
}

/* Functions to create object */
JNODE_p create_null(void) {
    JNODE_p node = new_node();
    node->type = J_NULL;
    return node;
}

JNODE_p create_true(void){
    JNODE_p node = new_node();
    node->type = J_True;
    return node;
}

JNODE_p create_false(void){
    JNODE_p node = new_node();
    node->type = J_False;
    return node;
}

JNODE_p create_int(int num) {
    JNODE_p node = new_node();
    node->type = J_Int;
    node->value.int_val = num;
    return node;
}

JNODE_p create_double(double num) {
    JNODE_p node = new_node();
    node->type = J_Double;
    node->value.double_val = num;
    return node;
}

JNODE_p create_string(const char *str) {
    JNODE_p node = new_node();
    node->type = J_String;
    node->value.string_val = print_const(str);
    return node;
}

JNODE_p create_array(void) {
    JNODE_p node = new_node();
    node->type = J_Array;
    return node;
}

JNODE_p create_object(void) {
    JNODE_p node = new_node();
    node->type = J_Object;
    return node;
}

/* Functions to create array with elements */
JNODE_p json_int_array(const int *nums, int len) {
    {
        JNODE_p next = NULL, prev = NULL, arr = create_array();
        for (int i = 0; arr && i < len; i++)
        {
            next = create_int(nums[i]);
            if (!i)
                arr->child = next;
            else {
                prev->next = next;
                next->prev = prev;
            }
            prev = next;
        }
        return arr;
    }
}

JNODE_p json_double_array(const double *nums, int len) {
    JNODE_p next = NULL, prev = NULL, arr = create_array();
    for (int i = 0; arr && i < len; i++)
    {
        next = create_double(nums[i]);
        if (!i)
            arr->child = next;
        else {
            prev->next = next;
            next->prev = prev;
        }
        prev = next;
    }
    return arr;
}

JNODE_p json_string_array(const char **strs, int len) {
    JNODE_p next = NULL, prev = NULL, arr = create_array();
    for (int i = 0; arr && i < len; i++)
    {
        next = create_string(strs[i]);
        if (!i)
            arr->child = next;
        else {
            prev->next = next;
            next->prev = prev;
        }
        prev = next;
    }
    return arr;
}

/* Functions to add node at object(with arr name) */
void json_add_null(JNODE_p obj, const char *name) {
    json_add_to_object(obj, name, create_null(), 1);
}

void json_add_true(JNODE_p obj, const char *name) {
    json_add_to_object(obj, name, create_true(), 1);
}

void json_add_false(JNODE_p obj, const char *name) {
    json_add_to_object(obj, name, create_false(), 1);
}

void json_add_int(JNODE_p obj, const char *name, int num) {
    json_add_to_object(obj, name, create_int(num), 1);
}

void json_add_double(JNODE_p obj, const char *name, double num) {
    json_add_to_object(obj, name, create_double(num), 1);
}

void json_add_string(JNODE_p obj, const char *name, char *str) {
    json_add_to_object(obj, name, create_string(str), 1);
}

/* Functions to add node at array or object(with arr name) */
void json_add_to_array(JNODE_p array, JNODE_p node) {
    JNODE_p c = array->child;
    if (!node) error_exit(5, "Add a empty node to array.\n");
    if (!c)
        array->child = node;
    else {
        while (c->next)
            c = c->next;
        c->next = node;
        node->prev = c;
    }
}

void json_add_to_object(JNODE_p object, const char *name, JNODE_p node, int on_heap) {
    if (!node) error_exit(5, "Add a empty node to object.\n");
    if (node->name && !(node->type & CONST_BIT))
        safe_free(node->name);
    if (on_heap)
        node->name = print_const(name);  // heap
    else {
        node->name = (char *)name;
        node->type |= CONST_BIT;
    }
    json_add_to_array(object, node);
}

/* Functions to delete node from array or object(with arr name) */
void json_del_from_array(JNODE_p array, int idx) {
    json_delete(json_detach_from_array(array, idx, 0));
}

void json_del_from_object(JNODE_p obj, const char *name) {
    json_delete(json_detach_from_object(obj, name));
}

// no_child = 1: when using this function at json_detach_from_object
JNODE_p json_detach_from_array(JNODE_p array, int idx, int no_child) {
    JNODE_p c = NULL;
    if (no_child) c = array;
    else c = array->child;
    while (c && idx > 0)
        c = c->next, idx--;
    if (!c) error_exit(6, "Failed to detach from json, index error.\n");
    if (c->prev)
        c->prev->next = c->next;
    if (c->next)
        c->next->prev = c->prev;
    if (!no_child && c == array->child)  // first child
        array->child = c->next;
    c->prev = c->next = NULL;
    return c;
}

JNODE_p json_detach_from_object(JNODE_p obj, const char *name) {
    int i = 0;
    JNODE_p c = obj, out = NULL;
    while (c && !out) {
        if (!strcmp_case(c->name, name)) {
            out = json_detach_from_array(obj, i, 1);
            return out;
        }
        if (c->child)
            out = json_detach_from_object(c->child, name);
        i++, c = c->next;
    }
    return out;
}

/* Functions to replace arr node in array or object(with arr name) */
// no_child = 1: when using this function at json_replace_object, otherwise 0.
void json_replace_array(JNODE_p array, int idx, JNODE_p newitem, int no_child) {
    JNODE_p c = NULL;
    if (no_child) c = array;
    else c = array->child;

    while (c && idx > 0)
        c = c->next, idx--;
    if (!c) error_exit(6, "Failed to replace from json, index error.\n");
    newitem->next = c->next;
    newitem->prev = c->prev;
    if (newitem->next)
        newitem->next->prev = newitem;
    if (!no_child && c == array->child)  // first child
        array->child = newitem;
    else if (newitem->prev)  // first node
        newitem->prev->next = newitem;
    c->next = c->prev = NULL;
    json_delete(c);
}

int json_replace_object(JNODE_p obj, const char *name, JNODE_p newitem) {
    int i = 0, flag = 0;
    JNODE_p c = obj;
    while (c && !flag) {
        if (!strcmp_case(c->name, name)) {
            newitem->name = print_const(name);
            (void)json_replace_array(obj, i, newitem, 1);
            return 1;
        }
        if (c->child)
            flag = json_replace_object(c->child, name, newitem);
        i++, c = c->next;
    }
    return flag;
}

/* Functions to find arr node by name*/
JNODE_p json_get(JNODE_p root, const char *name) {
    JNODE_p c = root, out = NULL;
    while (c && !out) {
        if (!strcmp_case(c->name, name)) {
            (void)show_search_result(c, name);
            out = c;
            return out;
        }
        if (c->child)
            out = json_get(c->child, name);
        c = c->next;
    }
    if (!out) {
        printf("Key [%s] not found. \n", name);
        return NULL;
    }
    return out;
}

static void show_search_result(JNODE_p node, const char *name) {
    switch (node->type) {
        case J_NULL: {
            printf("Key: [%s] , value: [%s]. \n", name, "null");
            break;
        }
        case J_False: {
            printf("Key: [%s] , value: [%s]. \n", name, "false");
            break;
        }
        case J_True: {
            printf("Key: [%s] , value: [%s]. \n", name, "true");
            break;
        }
        case J_Int: {
            printf("Key: [%s] , value: [%d]. \n", name, node->value.int_val);
            break;
        }
        case J_Double: {
            printf("Key: [%s] , value: [%lf]. \n", name, node->value.double_val);
            break;
        }
        case J_String: {
            printf("Key: [%s] , value: [%s]. \n", name, node->value.string_val);
            break;
        }
        default: {
            printf("Key: [%s] , can`t get value of this node. \n", name);
            break;
        }
    }
}

/**********************************************************************************/
/* Static functions */
static inline void error_exit(int status, const char *error_msg) {
    fputs(error_msg, stderr);
    exit(status);
}

static inline void *emalloc(size_t size) {
    void *prev = malloc(size);
    if (prev == NULL)
        error_exit(3, "Insufficient memory.");
    return prev;
}

static inline void safe_free(void *ptr) {
    free(ptr);
    ptr = NULL;
}

static JNODE_p new_node(void) {
    JNODE_p node = (JNODE_p)emalloc(sizeof(JNODE_t));
    if (node) memset(node, 0, sizeof(JNODE_t));
    return node;
}

/* If s1 == s2, return 0 */
static int strcmp_case(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return 1;
    for (; tolower(*s1) == tolower(*s2); ++s1, ++s2)
        if (*s1 == 0)
            return 0;
    return 1;
}


static const char *skip_invalid(const char *value) {
    while (value && *value && (unsigned char) *value <= 32)
        value++;
    return value;
}

static const char *parse_value(JNODE_p node, const char *value) {
    if (!strncmp(value, "null", 4)) {
        node->type = J_NULL;
        return value + 4;
    }
    if (!strncmp(value, "false", 5)) {
        node->type = J_False;
        node->value.int_val = 0;
        return value + 5;
    }
    if (!strncmp(value, "true", 4)) {
        node->type = J_True;
        node->value.int_val = 1;
        return value + 4;
    }
    if (*value == '\"') { return parse_string(node, value); }
    if (*value == '-' || (*value >= '0' && *value <= '9')) {
        return parse_number(node, value);
    }
    if (*value == '[') { return parse_array(node, value); }
    if (*value == '{') { return parse_object(node, value); }

    ep = value;
    return NULL;
}

static const char *parse_string(JNODE_p node, const char *value) {
    const char *ptr = value + 1;
    int len = 0;
    while (*ptr != '\"' && *ptr && ++len) {
        if (*ptr++ == '\\')
            ptr++;  /* Skip escaped quotes. \\ means \ in code. \\ in text is escaped quotes*/
    }

    char *out = (char *)emalloc(sizeof(char) * (len + 1));

    // scan
    char *scan = out;
    ptr = value + 1;
    while (*ptr != '\"' && *ptr) {
        if (*ptr != '\\') *scan++ = *ptr++;
        else {
            ptr++;
            switch (*ptr) {
                case 'b':
                    *scan++ = '\b';
                    break;
                case 'f':
                    *scan++ = '\f';
                    break;
                case 'n':
                    *scan++ = '\n';
                    break;
                case 'r':
                    *scan++ = '\r';
                    break;
                case 't':
                    *scan++ = '\t';
                    break;
                default:
                    *scan++ = *ptr;
                    break;
            }
            ptr++;
        }
    }
    *scan = 0; // end
    if (*ptr == '\"') ptr++;
    node->value.string_val = out;
    node->type = J_String;
    return ptr;
}

static const char *parse_number(JNODE_p node, const char *value) {
    double next = 0, sign = 1;

    if (*value == '-')
        sign = -1, value++;
    if (*value == '0')
        value++;

    if (*value >= '1' && *value <= '9') {
        do
            next = (next * 10.0) + (*value++ - '0');
        while (*value >= '0' && *value <= '9');
    }

    int scale = 0;
    int is_fraction = 0;
    if (*value == '.' && value[1] >= '0' && value[1] <= '9') // value[1] look ahead.
    {
        is_fraction = 1;
        value++;
        do
            next = (next * 10.0) + (*value++ - '0'), scale--;
        while (*value >= '0' && *value <= '9');
    }

    if (is_fraction) {
        next = sign * next * pow(10.0, scale);
        node->value.double_val = next;
        node->type = J_Double;
    } else {
        next = sign * next;
        node->value.int_val = (int) next;
        node->type = J_Int;
    }

    return value;
}

static const char *parse_array(JNODE_p node, const char *value) {
    JNODE_p child = NULL;

    node->type = J_Array;
    value = skip_invalid(value + 1);
    if (*value == ']')
        return value + 1; /* empty array. */

    node->child = child = new_node();
    value = skip_invalid(parse_value(child, skip_invalid(value)));

    while (*value == ',') {
        JNODE_p new_item = new_node();
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip_invalid(parse_value(child, skip_invalid(value + 1)));
    }

    if (*value == ']')
        return value + 1; /* end of array */
    else {
        perror("Incorrect format, without arr ']'.\n");
        ep = value; // without end correctly
        return NULL;
    }
}

static const char *parse_object(JNODE_p node, const char *value) {
    JNODE_p child;

    node->type = J_Object;
    value = skip_invalid(value + 1);
    if (*value == '}')
        return value + 1; /* empty array. */

    node->child = child = new_node();
    // Parse name
    value = skip_invalid(parse_string(child, skip_invalid(value)));
    if (!value) return NULL; // Not end yet
    child->name = child->value.string_val;
    child->value.string_val = 0;  // reset
    if (*value != ':') {
        perror("Incorrect format, without arr ':' before assignment.\n");
        ep = value;
        return NULL;
    }
    // parse value
    value = skip_invalid(parse_value(child, skip_invalid(value + 1)));
    if (!value) return NULL;

    while (*value == ',') {
        JNODE_p new_item = new_node();
        child->next = new_item;
        new_item->prev = child;
        // Parse name
        child = new_item;
        value = skip_invalid(parse_string(child, skip_invalid(value + 1)));
        if (!value) return NULL;
        child->name = child->value.string_val;
        child->value.string_val = 0;  // reset
        if (*value != ':') {
            perror("Incorrect format, without arr ':' before assignment.\n");
            ep = value;
            return 0;
        }
        // parse value
        value = skip_invalid(parse_value(child, skip_invalid(value + 1)));
        if (!value) return NULL;
    }

    if (*value == '}')
        return value + 1; /* end of object */
    else {
        perror("Incorrect format, without arr '}'.\n");
        ep = value; // without end correctly
        return NULL;
    }
}


static char *print_const(const char *str)
{
    size_t len;
    len = strlen(str) + 1;
    char *copy = (char *)emalloc(sizeof(char) * len);
    memcpy(copy, str, len);
    return copy;
}

static char *print_value(JNODE_p node, int depth) {
    char *out = 0;
    if (!node) return NULL;
    switch ((node->type) & 255) {
        case J_NULL:
            out = print_const("null");
            break;
        case J_False:
            out = print_const("false");
            break;
        case J_True:
            out = print_const("true");
            break;
        case J_Int:
            out = print_number(node);
            break;
        case J_Double:
            out = print_number(node);
            break;
        case J_String:
            out = print_string(node);
            break;
        case J_Array:
            out = print_array(node, depth);
            break;
        case J_Object:
            out = print_object(node, depth);
            break;
    }
    return out;
}


static char *print_number(JNODE_p node) {
    char *str = 0;
    int type = node->type;

    if (type == J_Int) {
        int num = node->value.int_val;
        if (num <= INT32_MAX && num >= INT32_MIN) {
            str = (char *)emalloc(sizeof(char) * 21);
            sprintf(str, "%d", node->value.int_val);
        } else {
            printf("Node %s : \n", node->name);
            error_exit(4, "Integer value overflow. \n");
        }
    }
    else if (type == J_Double) {
        double num = node->value.double_val;
        if (num <= DBL_MAX && num >= DBL_MIN) {
            str = (char *)emalloc(sizeof(char) * 64);
            sprintf(str, "%lf", node->value.double_val);
        } else {
            printf("Node %s : \n", node->name);
            error_exit(4, "Double value overflow. \n");
        }
    }

    return str;
}


static char *print_string_base(const char *str) {
    const char *ptr = NULL;
    char *out = NULL;

    if (!str) { // empty
        out = (char *)emalloc(sizeof(char) * 3);
        strcpy(out, "\"\"");
        return out;
    }

    int flag_escaped = 0;
    for (ptr = str; *ptr; ptr++) {
        if ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\'))
            flag_escaped = 1;
    }

    int len = 0;
    char *scan = NULL;
    if (!flag_escaped)  // Without escaped chars
    {
        len = ptr - str;
        out = (char *)emalloc(sizeof(char) * (len + 3));
        scan = out;
        *scan++ = '\"';
        strcpy(scan, str);
        scan[len] = '\"';
        scan[len + 1] = 0;
        return out;
    }

    // get length with escaped chars
    ptr = str;
    unsigned char token;
    while ((token = *ptr) && ++len)
    {
        if (strchr("\"\\\b\f\n\r\t", token))  // length add 1
            len++;
        else if (token < 32)
            len += 5;
        ptr++;
    }

    out = (char *)emalloc(sizeof(char) * (len + 3));
    scan = out;
    ptr = str;
    *scan++ = '\"';
    while (*ptr)
    {
        if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\')
            *scan++ = *ptr++;
        else
        {
            /* escape and print */
            *scan++ = '\\';
            switch (token = *ptr++)
            {
                case '\\':
                    *scan++ = '\\';
                    break;
                case '\"':
                    *scan++ = '\"';
                    break;
                case '\b':
                    *scan++ = 'b';
                    break;
                case '\f':
                    *scan++ = 'f';
                    break;
                case '\n':
                    *scan++ = 'n';
                    break;
                case '\r':
                    *scan++ = 'r';
                    break;
                case '\t':
                    *scan++ = 't';
                    break;
                default:
                    sprintf(scan, "u%04x", token);
                    scan += 5;
                    break;
            }
        }
    }
    *scan++ = '\"';
    *scan++ = 0;
    return out;
}

static char *print_string(JNODE_p node) {
    return print_string_base(node->value.string_val);
}


static char *print_array(JNODE_p node, int depth)
{
    /* How many entries in the array? */
    int num_child = 0;
    JNODE_p child = node->child;
    while (child)
        num_child++, child = child->next;
    /* Explicitly handle num_child==0 */
    char *out = 0;
    if (!num_child) {
        out = (char *)emalloc(sizeof(char) * 3);
        if (out)
            strcpy(out, "[]");
        return out;
    }

    /* Allocate an array to hold the values for each */
    char **entries;
    entries = (char **)emalloc(num_child * sizeof(char *));
    memset(entries, 0, num_child * sizeof(char *));

    /* Retrieve all the results: */
    int len = 5, i = 0, fail = 0;
    char *ret = NULL;
    child = node->child;
    while (child && !fail)
    {
        ret = print_value(child, depth + 1);
        entries[i++] = ret;
        if (ret)
            len += strlen(ret) + 3;
        else
            fail = 1;
        child = child->next;
    }

    /* Handle failure. */
    if (fail) {
        for (i = 0; i < num_child; i++)
            if (entries[i])
                safe_free(entries[i]);
        safe_free(entries);
        return 0;
    }

    /* Compose the output array. */
    out = (char *)emalloc(sizeof(char) * len);
    char *scan = out + 1;
    *scan = 0;
    *out = '[';
    size_t tmplen = 0;
    for (i = 0; i < num_child; i++) {
        tmplen = strlen(entries[i]);
        memcpy(scan, entries[i], tmplen);
        scan += tmplen;
        if (i != num_child - 1) {
            *scan++ = ',';
            *scan++ = ' ';
            *scan = 0;
        }
        safe_free(entries[i]);
    }
    safe_free(entries);
    *scan++ = ']';
    *scan++ = 0;
    return out;
}

static char *print_object(JNODE_p node, int depth)
{
    /* Count the number of entries. */
    int num_child = 0;
    JNODE_p child = node->child;
    while (child)
        num_child++, child = child->next;

    /* Explicitly handle empty object case */
    char *scan = NULL;
    char *out = NULL;
    if (!num_child) {
        out = (char *)emalloc(sizeof(char) * (depth + 5));
        scan = out;
        *scan++ = '{';
        *scan++ = '\n';
        for (int i = 0; i < depth - 1; i++)
            *scan++ = '\t';
        *scan++ = '}';
        *scan++ = 0;
        return out;
    }

    /* Allocate space for the names and the objects */
    char **entries = NULL, **names = NULL;
    entries = (char **)emalloc(num_child * sizeof(char *));
    names = (char **)emalloc(num_child * sizeof(char *));
    memset(entries, 0, sizeof(char *) * num_child);
    memset(names, 0, sizeof(char *) * num_child);

    /* Collect all the results into our arrays: */
    char *str = NULL, *ret = NULL;
    int len = 10, i = 0, fail = 0;
    child = node->child;
    depth++;
    len += depth;
    while (child && !fail) {
        names[i] = str = print_string_base(child->name);
        entries[i++] = ret = print_value(child, depth);
        if (str && ret)
            len += strlen(ret) + strlen(str) + 5 + depth;
        else
            fail = 1;
        child = child->next;
    }

    /* Handle failure */
    if (fail) {
        for (i = 0; i < num_child; i++)
        {
            if (names[i])
                safe_free(names[i]);
            if (entries[i])
                safe_free(entries[i]);
        }
        safe_free(names);
        safe_free(entries);
        return 0;
    }

    /* Compose the output: */
    size_t tmplen = 0;
    int j = 0;
    out = (char *)emalloc(sizeof(char) * len);
    *out = '{';
    scan = out + 1;
    *scan++ = '\n';
    *scan = 0;
    for (i = 0; i < num_child; i++)
    {
        for (j = 0; j < depth; j++)
            *scan++ = '\t';

        tmplen = strlen(names[i]);
        memcpy(scan, names[i], tmplen);

        scan += tmplen;
        *scan++ = ':';
        *scan++ = '\t';

        strcpy(scan, entries[i]);
        scan += strlen(entries[i]);
        if (i != num_child - 1)
            *scan++ = ',';
        *scan++ = '\n';
        *scan = 0;

        safe_free(names[i]);
        safe_free(entries[i]);
    }
    safe_free(names);
    safe_free(entries);
    for (i = 0; i < depth - 1; i++)
        *scan++ = '\t';
    *scan++ = '}';
    *scan++ = 0;
    return out;
}





