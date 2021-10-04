/*************************************************************************
	> File Name: include/cjson.h
	> Author: racle
	> Mail: racleray@qq.com
	> Created Time: Wed 25 Aug 2021 06:25:21 PM CST
 ************************************************************************/

#ifndef _INCLUDE_CJSON_H
#define _INCLUDE_CJSON_H

#include <ctype.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* JSON struct */
typedef struct JsonNode {
    struct JsonNode *next, *prev;
    struct JsonNode* child;

    int type;
    char* name; // node name

    union {
        char* string_val;
        int int_val;
        double double_val;
    } value;
} JNODE_t, *JNODE_p;


/* Functions for input and output */
JNODE_p text_to_json(char *);
JNODE_p read_json(char *);
void output_json(JNODE_p, FILE *);

/* Functions for parsing text to json */
JNODE_p json_parse(const char*);
void json_delete(JNODE_p);

/* Function for formating json struct, return char* to a print function */
char* json_format(JNODE_p);

/* Functions to create object */
JNODE_p create_null(void);
JNODE_p create_true(void);
JNODE_p create_false(void);
JNODE_p create_int(int);
JNODE_p create_double(double);
JNODE_p create_string(const char*);
JNODE_p create_array(void);
JNODE_p create_object(void);
/* Functions to create array with elements */
JNODE_p json_int_array(const int*, int);
JNODE_p json_double_array(const double*, int);
JNODE_p json_string_array(const char**, int);

/* Functions to add node at object(with a name) */
void json_add_null(JNODE_p, const char*);
void json_add_true(JNODE_p, const char*);
void json_add_false(JNODE_p, const char*);
void json_add_int(JNODE_p, const char*, int);
void json_add_double(JNODE_p, const char*, double);
void json_add_string(JNODE_p, const char*, char*);

/* Functions to add node at array or object(with a name) */
void json_add_to_array(JNODE_p, JNODE_p);
void json_add_to_object(JNODE_p, const char*, JNODE_p, int); // if on_heap == 0, don`t need free. stack memory.

/* Functions to delete node from array or object(with a name) */
void json_del_from_array(JNODE_p, int);
void json_del_from_object(JNODE_p, const char*);
JNODE_p json_detach_from_array(JNODE_p, int, int);
JNODE_p json_detach_from_object(JNODE_p, const char *);

/* Functions to replace a node in array or object(with a name) */
void json_replace_array(JNODE_p, int, JNODE_p, int);
int json_replace_object(JNODE_p, const char*, JNODE_p);

/* Functions to find a node by name*/
JNODE_p json_get(JNODE_p, const char *);

#endif

