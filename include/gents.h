#ifndef GENTS_H
#define GENTS_H

#include <stdio.h>
#include <stdint.h>

#define MAGIC_NUMBER "GENTS"
#define VERSION_NUMBER 1

#define MAX_FOREST_SIZE 23
#define MAX_CHILDREN 15
#define MAX_DEPTH 8
#define MAX_NAME_CHAR 52 
#define HASH_BYTE_LENGTH 7

struct serialized_child_node{
    uint32_t uid;
    char has_children;
};

struct serialized_node{
    char hash[7];
    uint32_t uid;
    char name[MAX_NAME_CHAR];
    uint8_t child_count;
    uint32_t ancestry_uids[MAX_DEPTH];
    struct serialized_child_node children[MAX_CHILDREN];
};

struct memory_node {
    uint32_t uid;
    uint32_t ancestry_uids[8];
    uint8_t children_count;
    char name[52];
    struct memory_node *children[15];
};

//struct node* build_tree_from_yaml(const char* filename);

struct nested_node* create_nested_node(const char* name);
int add_nested_child(struct nested_node* parent, struct nested_node* child);
void print_nested_tree(struct nested_node *root, int depth);
void free_nested_tree(struct nested_node *root);
struct nested_node* build_tree_from_yaml(const char* filename);

// Serialize a single tree to a binary file
int serialize_tree(FILE* file, struct nested_node* node);

#endif /* GENTS_H */
