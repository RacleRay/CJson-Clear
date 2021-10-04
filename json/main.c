/*************************************************************************
	> File Name: main.c
	> Author: racle
	> Mail: racleray@qq.com
	> Created Time: Sat 28 Aug 2021 10:51:31 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"cjson.h"


JNODE_p test_json_create(void) {
    // test 1
    int int_arr[] = {1,2,3};
    const char *s_arr[3] = {"Peter", "Alice", "Henry"};
    
    JNODE_p root = create_object();
    JNODE_p img = create_object();
    JNODE_p info = create_object();
    JNODE_p e_arr = json_int_array(int_arr, sizeof(int_arr) / sizeof(int));
    JNODE_p a_arr = json_string_array(s_arr, 3);
    // Add
    (void)json_add_to_object(root, "Image", img, 1);
    (void)json_add_int(img, "Width", 1024);
    (void)json_add_int(img, "Height", 768);
    (void)json_add_double(img, "Rating", 6.66);
    (void)json_add_string(img, "Title", "Example");
    (void)json_add_to_object(img, "Others", info, 1);
    (void)json_add_string(info, "Url", "https://www.haha.com/123.jpg");
    (void)json_add_true(info, "Legal");
    (void)json_add_to_object(img, "Array", e_arr, 1);
    (void)json_add_to_object(info, "Author", a_arr, 1);
    (void)output_json(root, stdout);

    // Delete 
    (void)json_del_from_object(root, "Array");
    (void)output_json(root, stdout);

    // Modify
    JNODE_p repl = create_string("New Title");
    json_replace_object(root, "Title", repl);
    (void)output_json(root, stdout);

    // Search
    JNODE_p target = NULL;
    target = json_get(root, "Url");
    if (!target)
        printf("No result");

    return root;
}

void test_json_to_file(JNODE_p root, char *filename) {
    FILE *out = fopen(filename, "wb");
    if (out) {
        output_json(root, out);
    }
    fclose(out);
    return;
}

JNODE_p test_json_read(char *filename) {
    FILE *in = fopen(filename, "rb");
    JNODE_p root = NULL;
    if (in)
        root = read_json(filename);
    if (root == NULL)
        puts("Failed to read json file .\n");
    fclose(in);
    return root;
}


int main() {
    JNODE_p root = NULL;
    root = test_json_create();

    (void)test_json_to_file(root, "json_test.json");

    json_delete(root);

    JNODE_p root_read = NULL;
    root_read = test_json_read("json_test.json");
    char *formated = json_format(root_read);
    printf("\n%s\n", formated);

    free(formated);
    formated = NULL;
    json_delete(root_read);

    return 0;
}
